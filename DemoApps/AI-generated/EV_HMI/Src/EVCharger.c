/**
 * @file EVCharger.c
 * @brief 800x480 EV charger HMI for BT817 and Raspberry Pi RP2040.
 */
#include <stdio.h>
#include <string.h>
#include "Common.h"
#include "Platform.h"
#include "EVE_CoCmd.h"
#include "EVCharger.h"
#include "Maths.h"

#define RGB_BASE     0x101418UL
#define RGB_SURFACE  0x1A2028UL
#define RGB_LINE     0x2C3540UL
#define RGB_TEXT     0xF2F4F6UL
#define RGB_DIM      0x9AA4B0UL
#define RGB_BLUE     0x3B8BEBUL
#define RGB_GREEN    0x2FBF71UL
#define RGB_AMBER    0xF0B23CUL
#define RGB_RED      0xE5483FUL

#define TAG_SETTINGS 1
#define TAG_LANGUAGE 2
#define TAG_WELCOME 3
#define TAG_CANCEL   10
#define TAG_STOP     11
#define TAG_MODAL_CANCEL 12
#define TAG_MODAL_STOP   13
#define TAG_DONE     14
#define TAG_RETRY    15

#define TOUCH_DOWN_MS 50U
#define TOUCH_UP_MS 30U
#define PAGE_LOCK_MS 200U
#define MODAL_LOCK_MS 300U
#define STOP_HOLD_MS 1500U
#define SETTINGS_HOLD_MS 5000U

#define FONT24_HANDLE 1
#define FONT32_HANDLE 2
#define FONT48_HANDLE 3
#define FONT24_RAMG   0UL
#define FONT32_RAMG   6000UL
#define FONT48_RAMG   12000UL
#define FONT24_FLASH  4288UL
#define FONT32_FLASH  463360UL
#define FONT48_FLASH  1738048UL
#define XFONT_COPY_SIZE 5944UL
#define FONT_CACHE_RAMG 32768UL
#define FONT_CACHE_SIZE (512UL * 1024UL)
#define UI_DESIGN_WIDTH 800L

typedef struct UiState {
    EvChargerPage page;
    EvChargerPage page_before_fault;
    EvChargerPage page_before_settings;
    EvChargerInputs in;
    uint32_t now;
    uint32_t entered;
    uint32_t last_touch_change;
    uint32_t touch_started;
    uint8_t raw_tag;
    uint8_t stable_tag;
    uint8_t released_tag;
    bool stop_modal;
    bool english;
    bool stop_sent;
    bool settings_unlocked;
    uint8_t pin_length;
    char pin[5];
    bool fonts_ready;
    uint32_t last_activity;
    uint32_t filtered_lux;
    bool previous_plug;
    bool ignore_plug_until_removed;
    uint8_t pwm_duty;
#if EVE_SUPPORT_GEN >= EVE4
    uint32_t last_underrun;
    uint32_t last_underrun_check;
#endif
} UiState;

static EVE_HalContext s_hal;
static UiState s_ui;

#if defined(EV_CHARGER_DEMO)
static bool s_demo_stopped;
static uint32_t s_demo_meter_age;
static bool s_demo_started;
static uint32_t s_demo_started_at;
#endif

static int16_t ui_x(int32_t x)
{
    return (int16_t)((x * (int32_t)s_hal.Width + UI_DESIGN_WIDTH / 2L) / UI_DESIGN_WIDTH);
}

static int16_t ui_x16(int32_t x16)
{
    return (int16_t)((x16 * (int32_t)s_hal.Width + UI_DESIGN_WIDTH / 2L) / UI_DESIGN_WIDTH);
}

static void log_display_config(void)
{
    printf("EVCharger display freq=%lu h=%u/%u hsf=%lu v=%u/%u pclk=%u pclkfreq=0x%04lx\n",
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_FREQUENCY),
        EVE_Hal_rd16(&s_hal, REG_HSIZE), EVE_Hal_rd16(&s_hal, REG_HCYCLE),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_HSF_HSIZE),
        EVE_Hal_rd16(&s_hal, REG_VSIZE), EVE_Hal_rd16(&s_hal, REG_VCYCLE),
        EVE_Hal_rd8(&s_hal, REG_PCLK),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_PCLK_FREQ));
}

#if defined(__GNUC__)
__attribute__((weak))
#endif
void EVCharger_ReadInputs(EvChargerInputs *in)
{
#if defined(EV_CHARGER_DEMO)
    uint32_t age;
    age = s_demo_started ? 5U + (EVE_millis() - s_demo_started_at) / 1000U : 0U;
    /* Latch the last meter sample on manual stop or automatic completion. */
    if (!s_demo_stopped) {
        s_demo_meter_age = age < 45U ? age : 45U;
        if (age >= 45U) s_demo_stopped = true;
    }
    memset(in, 0, sizeof(*in));
    in->plug_connected = age >= 5U;
    in->authorized = age >= 10U;
    in->charge_complete = s_demo_stopped;
    in->power_kw = s_demo_stopped ? 0U : 58U;
    in->soc_percent = (uint16_t)(62U + ((s_demo_meter_age / 10U) % 20U));
    in->energy_deci_kwh = 124U + s_demo_meter_age / 6U;
    in->cost_cents = 1488U + s_demo_meter_age * 2U;
    in->elapsed_seconds = s_demo_meter_age;
    in->ambient_lux = 1200;
    in->hour = 14;
    in->minute = (uint8_t)(32U + ((age / 60U) % 28U));
    in->touch_allowed = true;
    in->fault_retryable = true;
    in->fault_code = "E-021";
    in->fault_message_en = "CONNECTOR NOT LOCKED - REINSERT";
    in->fault_message_zh = "充电枪未锁定, 请重新插入";
#else
    memset(in, 0, sizeof(*in));
    in->ambient_lux = 5000;
    in->touch_allowed = true;
#endif
}

#if defined(__GNUC__)
__attribute__((weak))
#endif
void EVCharger_RequestStop(void)
{
#if defined(EV_CHARGER_DEMO)
    s_demo_stopped = true;
#endif
}

#if defined(__GNUC__)
__attribute__((weak))
#endif
void EVCharger_RequestCancelAuthorization(void) { }

#if defined(__GNUC__)
__attribute__((weak))
#endif
void EVCharger_RequestRetry(void) { }

#if defined(__GNUC__)
__attribute__((weak))
#endif
void EVCharger_Beep(uint16_t milliseconds) { (void)milliseconds; }

#if defined(__GNUC__)
__attribute__((weak))
#endif
bool EVCharger_GetQrRow(uint8_t row, uint64_t *module_bits)
{
    /* QR version 6-M: This is an EV charger point demo. */
    static const uint64_t demo_rows[41] = {
        0x1fcc01b937fULL,
        0x105518a4041ULL,
        0x175fb21da5dULL,
        0x175bd66bf5dULL,
        0x175974efd5dULL,
        0x104ce72ad41ULL,
        0x1fd5555557fULL,
        0x60bff100ULL,
        0x13fa973b9d1ULL,
        0xc86ea46c10ULL,
        0x19e5882dacfULL,
        0x1340447bb21ULL,
        0x21974c716bULL,
        0x7ac01b0d26ULL,
        0x6ae7528f3ULL,
        0xb7fb20360dULL,
        0x1a2429977c7ULL,
        0x175b76df1a3ULL,
        0x1b1518a99e9ULL,
        0x18804dea61bULL,
        0xd19f452eddULL,
        0x8ce21b973cULL,
        0x15eca33627fULL,
        0x175bd27af95ULL,
        0x7970eab370ULL,
        0x3354c71f11ULL,
        0x12fe58de7ULL,
        0xaaeaa05915ULL,
        0x1a6c21959e6ULL,
        0xd9b76dd291ULL,
        0x183518a4378ULL,
        0x12404df4cb0ULL,
        0x3f9f44145bULL,
        0x191c01a2700ULL,
        0x1552e75fb7fULL,
        0xf1fb215641ULL,
        0x1ff4298c35dULL,
        0x16a954c305dULL,
        0x1a1b1c8145dULL,
        0x18020fb5e41ULL,
        0xc58933a57fULL,
    };
    if (row >= 41U || !module_bits) return false;
    *module_bits = demo_rows[row];
    return true;
}

static void color(uint32_t c)
{
    EVE_CoDl_colorRgb(&s_hal, (uint8_t)(c >> 16), (uint8_t)(c >> 8), (uint8_t)c);
}

static void rect(int16_t x, int16_t y, int16_t w, int16_t h, uint32_t c)
{
    color(c);
    EVE_CoDl_begin(&s_hal, RECTS);
    EVE_CoDl_vertex2f(&s_hal, VP(ui_x(x)), VP(y));
    EVE_CoDl_vertex2f(&s_hal, VP(ui_x((int32_t)x + w)), VP(y + h));
    EVE_CoDl_end(&s_hal);
}

static void line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t width, uint32_t c)
{
    color(c);
    EVE_CoDl_lineWidth(&s_hal, width * 16U);
    EVE_CoDl_begin(&s_hal, LINES);
    EVE_CoDl_vertex2f(&s_hal, VP(ui_x(x1)), VP(y1));
    EVE_CoDl_vertex2f(&s_hal, VP(ui_x(x2)), VP(y2));
    EVE_CoDl_end(&s_hal);
}

static void text(int16_t x, int16_t y, int16_t font, uint16_t opts, uint32_t c, const char *s)
{
    const unsigned char *scan = (const unsigned char *)s;
    while (*scan && *scan < 0x80U) ++scan;
    if (*scan && !s_ui.english && s_ui.fonts_ready) {
        /* BT817 provides one font cache. A single cached CJK font avoids
           competing external-flash reads from 24/32/48 px fonts per line. */
        font = FONT32_HANDLE;
    }
    color(c);
    EVE_CoCmd_text(&s_hal, ui_x(x), y, font, opts, s);
}

static const char *tr(const char *en, const char *zh)
{
    return (!s_ui.english && s_ui.fonts_ready) ? zh : en;
}

static void number_text(int16_t x, int16_t y, int16_t font, uint16_t opts, uint32_t c,
    const char *fmt, uint32_t value)
{
    char b[32];
    snprintf(b, sizeof(b), fmt, (unsigned long)value);
    text(x, y, font, opts, c, b);
}

static uint32_t pressed_color(uint8_t tag, uint32_t c)
{
    uint32_t r, g, b;
    if (s_ui.stable_tag != tag) return c;
    r = (c >> 16) & 0xFFU; g = (c >> 8) & 0xFFU; b = c & 0xFFU;
    r += (255U - r) * 15U / 100U;
    g += (255U - g) * 15U / 100U;
    b += (255U - b) * 15U / 100U;
    return (r << 16) | (g << 8) | b;
}

static void tag_hitbox(uint8_t tag, int16_t x, int16_t y, int16_t w, int16_t h)
{
    EVE_CoDl_tag(&s_hal, tag);
    EVE_CoDl_colorMask(&s_hal, false, false, false, false);
    rect(x, y, w, h, RGB_BASE);
    EVE_CoDl_colorMask(&s_hal, true, true, true, true);
}

static void button(uint8_t tag, int16_t x, int16_t y, int16_t w, int16_t h,
    uint32_t fill, uint32_t fg, const char *label)
{
    EVE_CoDl_tag(&s_hal, tag);
    rect(x, y, w, h, pressed_color(tag, fill));
    text(x + w / 2, y + h / 2, 29, OPT_CENTER, fg, label);
    EVE_CoDl_tag(&s_hal, 0);
}

static void outline_button(uint8_t tag, int16_t x, int16_t y, int16_t w, int16_t h,
    uint32_t stroke, const char *label)
{
    uint16_t width = s_ui.stable_tag == tag ? 3U : 2U;
    tag_hitbox(tag, x, y, w, h);
    stroke = pressed_color(tag, stroke);
    rect(x, y, w, width, stroke); rect(x, y + h - width, w, width, stroke);
    rect(x, y, width, h, stroke); rect(x + w - width, y, width, h, stroke);
    text(x + w / 2, y + h / 2, 29, OPT_CENTER, stroke, label);
    EVE_CoDl_tag(&s_hal, 0);
}

static const char *title_for_page(void)
{
    static const char *const en[] = { "WELCOME", "SCAN OR TAP", "CHARGING", "COMPLETE", "CHARGING STOPPED", "SETTINGS" };
    static const char *const zh[] = { "欢迎使用", "扫码或刷卡", "充电中", "充电完成", "充电中断", "维护设置" };
    return (s_ui.english || !s_ui.fonts_ready ? en : zh)[s_ui.page];
}

static void load_flash_fonts(void)
{
#if defined(EVE_SUPPORT_FLASH)
    if (!FlashHelper_SwitchFullMode(&s_hal)) return;
    EVE_CoCmd_flashRead(&s_hal, FONT24_RAMG, FONT24_FLASH, XFONT_COPY_SIZE);
    EVE_CoCmd_flashRead(&s_hal, FONT32_RAMG, FONT32_FLASH, XFONT_COPY_SIZE);
    EVE_CoCmd_flashRead(&s_hal, FONT48_RAMG, FONT48_FLASH, XFONT_COPY_SIZE);
    EVE_Cmd_waitFlush(&s_hal);
    EVE_CoCmd_setFont2(&s_hal, FONT24_HANDLE, FONT24_RAMG, 0);
    EVE_CoCmd_setFont2(&s_hal, FONT32_HANDLE, FONT32_RAMG, 0);
    EVE_CoCmd_setFont2(&s_hal, FONT48_HANDLE, FONT48_RAMG, 0);
#if EVE_SUPPORT_GEN >= EVE4
    /* BT817 has one font cache. Cache the 32 px body font, which dominates
       the localized pages and is otherwise fetched from flash every scanline. */
    EVE_CoCmd_fontCache(&s_hal, FONT32_HANDLE, FONT_CACHE_RAMG, FONT_CACHE_SIZE);
#endif
    EVE_Cmd_waitFlush(&s_hal);
    s_ui.fonts_ready = true;
#endif
}

static void topbar(void)
{
    char clock_text[8];
    rect(0, 0, 800, 70, RGB_SURFACE);
    line(0, 69, 800, 69, 1, RGB_LINE);
    tag_hitbox(TAG_SETTINGS, 20, 5, 140, 60);
    text(30, 35, 27, OPT_CENTERY, RGB_DIM, "A-07");
    EVE_CoDl_tag(&s_hal, 0);
    color(s_ui.page == EV_PAGE_FAULT ? RGB_RED : (s_ui.page == EV_PAGE_CHARGING ? RGB_GREEN : RGB_BLUE));
    EVE_CoDl_pointSize(&s_hal, 12U * 16U);
    EVE_CoDl_begin(&s_hal, POINTS); EVE_CoDl_vertex2f(&s_hal, VP(ui_x(112)), VP(35)); EVE_CoDl_end(&s_hal);
    color(RGB_BLUE); EVE_CoDl_begin(&s_hal, POINTS); EVE_CoDl_vertex2f(&s_hal, VP(ui_x(134)), VP(35)); EVE_CoDl_end(&s_hal);
    text(400, 35, 29, OPT_CENTER, RGB_TEXT, title_for_page());
    tag_hitbox(TAG_LANGUAGE, 590, 5, 60, 60);
    text(620, 35, 26, OPT_CENTER, RGB_DIM, s_ui.english ? "EN" : "ZH");
    EVE_CoDl_tag(&s_hal, 0);
    snprintf(clock_text, sizeof(clock_text), "%02u:%02u", s_ui.in.hour, s_ui.in.minute);
    text(735, 35, 27, OPT_CENTER, RGB_DIM, clock_text);
}

static void plug_icon(int16_t x, int16_t y, uint32_t c)
{
    line(x + 30, y + 20, x + 90, y + 20, 5, c);
    line(x + 30, y + 20, x + 30, y + 90, 5, c);
    line(x + 90, y + 20, x + 90, y + 90, 5, c);
    line(x + 30, y + 90, x + 90, y + 90, 5, c);
    line(x + 45, y + 90, x + 45, y + 110, 5, c);
    line(x + 75, y + 90, x + 75, y + 110, 5, c);
    line(x + 43, y + 48, x + 77, y + 48, 4, c);
    line(x + 43, y + 66, x + 77, y + 66, 4, c);
}

static void page_idle(void)
{
    int16_t dy = (int16_t)(((s_ui.now / 750U) & 1U) ? 8 : 0);
    tag_hitbox(TAG_WELCOME, 330, 85, 140, 140);
    plug_icon(340, (int16_t)(92 + dy), pressed_color(TAG_WELCOME, RGB_BLUE));
    EVE_CoDl_tag(&s_hal, 0);
    text(400, 258, 31, OPT_CENTER, RGB_TEXT, tr("INSERT CHARGING CONNECTOR", "请插入充电枪"));
    rect(30, 320, 360, 120, RGB_SURFACE);
    text(60, 348, 27, 0, RGB_DIM, tr("ENERGY PRICE", "电价"));
    text(60, 405, 29, OPT_CENTERY, RGB_TEXT, tr("1.20 CNY/kWh", "1.20 ¥/kWh"));
    rect(410, 320, 360, 120, RGB_SURFACE);
    text(440, 348, 27, 0, RGB_DIM, tr("MAX POWER", "最大功率"));
    text(440, 405, 29, OPT_CENTERY, RGB_TEXT, "120 kW");
    text(400, 462, 26, OPT_CENTER, RGB_DIM, tr("WeChat Pay | Alipay | RFID card", "QR | ALIPAY | 充电卡"));
}

static void draw_qr(void)
{
    uint8_t row, col;
    uint64_t rows[41];
    bool has_qr = true;
    for (row = 0; row < 41; ++row) {
        if (!EVCharger_GetQrRow(row, &rows[row])) { has_qr = false; break; }
    }
    rect(70, 110, 260, 260, 0xFFFFFFUL);
    if (!has_qr) return;
    color(0x000000UL);
    EVE_CoDl_lineWidth(&s_hal, 8U);
    EVE_CoDl_begin(&s_hal, RECTS);
    for (row = 0; row < 41; ++row) {
        for (col = 0; col < 41; ++col) {
            bool bit = (rows[row] & (1ULL << col)) != 0U;
            if (bit) {
                uint8_t first = col;
                int16_t y = (int16_t)(137 + row * 5);
                while (col + 1U < 41U && (rows[row] & (1ULL << (col + 1U)))) ++col;
                /* Five pixels/module with a four-module white quiet zone. */
                EVE_CoDl_vertex2f(&s_hal, ui_x16((97 + first * 5) * 16 + 8), y * 16 + 8);
                EVE_CoDl_vertex2f(&s_hal, ui_x16((97 + (col + 1) * 5) * 16 - 8), (y + 5) * 16 - 8);
            }
        }
    }
    EVE_CoDl_end(&s_hal);
}

static void countdown_arc(uint32_t remaining_ms)
{
    uint32_t step;
    uint32_t sweep = remaining_ms * 65536UL / 60000UL;
    if (!remaining_ms) return;
    color(RGB_AMBER);
    EVE_CoDl_lineWidth(&s_hal, 4U * 16U);
    EVE_CoDl_begin(&s_hal, LINE_STRIP);
    for (step = 0; step <= 60U; ++step) {
        uint16_t angle = (uint16_t)(sweep * step / 60U);
        int16_t x = (int16_t)(600 * 16 + (36L * 16 * Math_Qsin(angle)) / 32767L);
        int16_t y = (int16_t)(330 * 16 - (36L * 16 * Math_Qcos(angle)) / 32767L);
        EVE_CoDl_vertex2f(&s_hal, ui_x16(x), y);
    }
    EVE_CoDl_end(&s_hal);
}

static void page_connected(void)
{
    uint32_t elapsed_ms = s_ui.now - s_ui.entered;
    uint32_t remaining_ms = elapsed_ms < 60000U ? 60000U - elapsed_ms : 0U;
    uint32_t remain = (remaining_ms + 999U) / 1000U;
    draw_qr();
    line(400, 70, 400, 480, 1, RGB_LINE);
    text(200, 405, 27, OPT_CENTER, RGB_DIM, tr("SCAN WITH WECHAT / ALIPAY", "QR / ALIPAY 扫码"));
    rect(550, 120, 100, 60, RGB_AMBER);
    rect(560, 132, 80, 8, RGB_BASE);
    text(600, 245, 29, OPT_CENTER, RGB_TEXT, tr("OR TAP RFID CARD", "或刷卡"));
    color(RGB_LINE); EVE_CoDl_pointSize(&s_hal, 40U * 16U);
    EVE_CoDl_begin(&s_hal, POINTS); EVE_CoDl_vertex2f(&s_hal, VP(ui_x(600)), VP(330)); EVE_CoDl_end(&s_hal);
    color(RGB_SURFACE); EVE_CoDl_pointSize(&s_hal, 32U * 16U);
    EVE_CoDl_begin(&s_hal, POINTS); EVE_CoDl_vertex2f(&s_hal, VP(ui_x(600)), VP(330)); EVE_CoDl_end(&s_hal);
    countdown_arc(remaining_ms);
    number_text(600, 330, 28, OPT_CENTER, RGB_TEXT, "%lu", remain);
    outline_button(TAG_CANCEL, 530, 390, 240, 70, RGB_LINE, tr("CANCEL", "取消"));
}

static void graphic_dot(int16_t x, int16_t y, uint16_t radius, uint32_t c)
{
    color(c);
    EVE_CoDl_pointSize(&s_hal, radius * 16U);
    EVE_CoDl_begin(&s_hal, POINTS);
    EVE_CoDl_vertex2f(&s_hal, VP(ui_x(x)), VP(y));
    EVE_CoDl_end(&s_hal);
}

static void car_icon(void)
{
    bool flowing = !s_ui.stop_sent && !s_ui.in.charge_complete && s_ui.in.power_kw > 0U;
    uint32_t accent = flowing ? RGB_GREEN : RGB_DIM;
    uint8_t i;
    /* Dedicated illustration card, clear of the values and Stop button. */
    EVE_CoDl_saveContext(&s_hal);
    EVE_CoDl_tag(&s_hal, 0);
    EVE_CoDl_lineWidth(&s_hal, 16U);
    rect(520, 100, 250, 230, RGB_SURFACE);
    line(536, 250, 754, 250, 1, RGB_LINE);

    /* Charger pedestal with a recessed display and lightning symbol. */
    EVE_CoDl_lineWidth(&s_hal, 16U);
    rect(536, 140, 46, 105, 0x344454UL);
    rect(541, 145, 36, 62, 0x101C27UL);
    rect(546, 150, 26, 4, accent);
    line(562, 164, 552, 180, 2, accent);
    line(552, 180, 563, 180, 2, accent);
    line(563, 180, 555, 195, 2, accent);
    graphic_dot(559, 222, 4, accent);
    line(533, 247, 585, 247, 3, RGB_DIM);

    /* Side-profile EV: roof, dark glazing, metallic body and wheels. */
    line(618, 206, 635, 184, 3, 0xAFC6DDUL);
    line(635, 184, 696, 184, 3, 0xAFC6DDUL);
    line(696, 184, 719, 207, 3, 0xAFC6DDUL);
    line(634, 200, 699, 200, 5, 0x25465FUL);
    line(664, 187, 664, 208, 1, 0xAFC6DDUL);
    line(610, 217, 737, 217, 10, 0xAFC6DDUL);
    line(613, 229, 735, 229, 6, 0x657E98UL);
    line(677, 213, 686, 213, 1, RGB_TEXT);
    line(730, 215, 739, 215, 2, 0xF2F4F6UL);
    line(606, 215, 611, 215, 2, RGB_RED);
    for (i = 0; i < 2; ++i) {
        int16_t x = (int16_t)(633 + i * 79);
        graphic_dot(x, 238, 12, RGB_BASE);
        graphic_dot(x, 238, 7, 0x8197AEUL);
        graphic_dot(x, 238, 3, RGB_SURFACE);
    }

    /* Cable path: charger -> lower loop -> vehicle charge port. */
    line(582, 216, 592, 216, 3, RGB_LINE);
    line(592, 216, 592, 275, 3, RGB_LINE);
    line(592, 275, 610, 275, 3, RGB_LINE);
    line(610, 275, 610, 222, 3, RGB_LINE);
    graphic_dot(610, 222, 5, accent);
    if (flowing) {
        /* Constant-speed pulses along the 140-pixel cable; stop on Stop. */
        for (i = 0; i < 3; ++i) {
            uint32_t d = (s_ui.now / 16U + i * 47U) % 140U;
            int16_t x, y;
            if (d < 10U) { x = (int16_t)(582 + d); y = 216; }
            else if (d < 69U) { x = 592; y = (int16_t)(216 + d - 10U); }
            else if (d < 87U) { x = (int16_t)(592 + d - 69U); y = 275; }
            else { x = 610; y = (int16_t)(275 - (d - 87U)); }
            graphic_dot(x, y, 3, accent);
        }
    }
    text(645, 308, 26, OPT_CENTER, accent,
        flowing ? tr("ENERGY FLOW", "充电中") : tr("STOPPING...", "停止中..."));
    EVE_CoDl_restoreContext(&s_hal);
}

static void page_charging(void)
{
    char b[48];
    snprintf(b, sizeof(b), "%lu.%lu kWh", (unsigned long)(s_ui.in.energy_deci_kwh / 10U), (unsigned long)(s_ui.in.energy_deci_kwh % 10U));
    text(30, 140, 31, OPT_CENTERY, RGB_TEXT, b);
    text(30, 205, 27, OPT_CENTERY, RGB_DIM, tr("ENERGY DELIVERED", "已充电量"));
    text(30, 255, 26, 0, RGB_DIM, tr("POWER", "功率")); number_text(30, 302, 29, OPT_CENTERY, RGB_TEXT, "%lu kW", s_ui.in.power_kw);
    text(190, 255, 26, 0, RGB_DIM, tr("DURATION", "时长"));
    snprintf(b, sizeof(b), "%lu:%02lu", (unsigned long)(s_ui.in.elapsed_seconds / 60U), (unsigned long)(s_ui.in.elapsed_seconds % 60U));
    text(190, 302, 29, OPT_CENTERY, RGB_TEXT, b);
    text(350, 255, 26, 0, RGB_DIM, tr("COST", "费用"));
    snprintf(b, sizeof(b), "CNY %lu.%02lu", (unsigned long)(s_ui.in.cost_cents / 100U), (unsigned long)(s_ui.in.cost_cents % 100U));
    text(350, 302, 29, OPT_CENTERY, RGB_TEXT, b);
    rect(30, 350, 470, 24, RGB_LINE); rect(30, 350, (int16_t)(470U * s_ui.in.soc_percent / 100U), 24, RGB_GREEN);
    number_text(30, 408, 27, OPT_CENTERY, RGB_DIM, "BATTERY %lu%%", s_ui.in.soc_percent);
    car_icon();
    outline_button(TAG_STOP, 520, 360, 250, 100, s_ui.stop_sent ? RGB_DIM : RGB_RED,
        s_ui.stop_sent ? tr("STOPPING...", "停止中...") : tr("STOP CHARGING", "停止充电"));
}

static void page_summary(void)
{
    char b[40];
    if (s_ui.in.plug_connected) { rect(0, 70, 800, 40, RGB_AMBER); text(400, 90, 27, OPT_CENTER, RGB_BASE, tr("REMOVE CHARGING CONNECTOR", "请拔出充电枪")); }
    rect(100, 130, 600, 220, RGB_SURFACE);
    text(140, 170, 27, 0, RGB_DIM, tr("ENERGY", "充电量"));
    snprintf(b, sizeof(b), "%lu.%lu kWh", (unsigned long)(s_ui.in.energy_deci_kwh / 10U), (unsigned long)(s_ui.in.energy_deci_kwh % 10U)); text(660, 170, 29, OPT_RIGHTX, RGB_TEXT, b);
    text(140, 226, 27, 0, RGB_DIM, tr("DURATION", "时长")); snprintf(b, sizeof(b), "%lu:%02lu", (unsigned long)(s_ui.in.elapsed_seconds / 60U), (unsigned long)(s_ui.in.elapsed_seconds % 60U)); text(660, 226, 29, OPT_RIGHTX, RGB_TEXT, b);
    line(140, 260, 660, 260, 1, RGB_LINE);
    text(140, 300, 27, 0, RGB_DIM, tr("TOTAL | 1.20 CNY/kWh", "费用 | 1.20 ¥/kWh")); snprintf(b, sizeof(b), "CNY %lu.%02lu", (unsigned long)(s_ui.in.cost_cents / 100U), (unsigned long)(s_ui.in.cost_cents % 100U)); text(660, 300, 31, OPT_RIGHTX, RGB_TEXT, b);
    button(TAG_DONE, 250, 380, 300, 90, RGB_BLUE, RGB_TEXT, tr("DONE", "完成"));
}

static void page_fault(void)
{
    const char *message = tr(s_ui.in.fault_message_en ? s_ui.in.fault_message_en : "DEVICE FAULT - CONTACT SUPPORT",
        s_ui.in.fault_message_zh ? s_ui.in.fault_message_zh : "设备故障, 客服 400-000-0000");
    rect(0, 0, 800, 8, RGB_RED); rect(0, 472, 800, 8, RGB_RED); rect(0, 0, 8, 480, RGB_RED); rect(792, 0, 8, 480, RGB_RED);
    line(400, 105, 355, 190, 6, RGB_RED); line(400, 105, 445, 190, 6, RGB_RED); line(355, 190, 445, 190, 6, RGB_RED);
    text(400, 155, 30, OPT_CENTER, RGB_RED, "!");
    text(400, 245, 31, OPT_CENTER, RGB_TEXT, message);
    text(400, 295, 27, OPT_CENTER, RGB_DIM, s_ui.in.fault_code ? s_ui.in.fault_code : "E-021");
    if (s_ui.in.fault_retryable)
        button(TAG_RETRY, 250, 330, 300, 100, RGB_BLUE, RGB_TEXT, tr("RETRY", "重试"));
    text(400, 455, 26, OPT_CENTER, RGB_DIM, tr("SUPPORT 400-000-0000", "客服 400-000-0000"));
}

static void page_settings(void)
{
    static const char *const keys[12] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "CLR", "0", "OK" };
    uint8_t i;
    if (!s_ui.settings_unlocked) {
        char mask[16] = "-  -  -  -";
        uint8_t m;
        text(400, 105, 29, OPT_CENTER, RGB_TEXT, tr("ENTER SERVICE PIN", "维护 PIN"));
        for (m = 0; m < s_ui.pin_length && m < 4U; ++m) mask[m * 3U] = '*';
        text(400, 145, 27, OPT_CENTER, RGB_DIM, mask);
        for (i = 0; i < 12; ++i) {
            int16_t x = (int16_t)(250 + (i % 3U) * 100);
            int16_t y = (int16_t)(175 + (i / 3U) * 72);
            button((uint8_t)(20U + i), x, y, 90, 62, RGB_SURFACE, RGB_TEXT, keys[i]);
        }
        return;
    }
    rect(30, 90, 250, 350, RGB_SURFACE);
    text(60, 130, 27, 0, RGB_TEXT, tr("NETWORK", "网络")); text(60, 180, 27, 0, RGB_TEXT, tr("ENERGY PRICE", "电价"));
    text(60, 230, 27, 0, RGB_TEXT, tr("LANGUAGE", "语言")); text(60, 280, 27, 0, RGB_TEXT, tr("BRIGHTNESS", "亮度"));
    text(60, 330, 27, 0, RGB_TEXT, tr("SELF TEST", "自检")); text(60, 380, 27, 0, RGB_TEXT, tr("LOGS", "日志"));
    text(500, 150, 29, OPT_CENTER, RGB_TEXT, tr("LANGUAGE", "语言"));
    button(TAG_LANGUAGE, 360, 200, 280, 80, RGB_BLUE, RGB_TEXT, s_ui.english ? "ENGLISH" : "ZH");
    text(500, 325, 27, OPT_CENTER, RGB_DIM, tr("Long-press station ID to exit", "设置 返回"));
}

static void stop_modal(void)
{
    char b[80];
    EVE_CoDl_colorA(&s_hal, 180); rect(0, 0, 800, 480, 0x000000UL); EVE_CoDl_colorA(&s_hal, 255);
    rect(150, 120, 500, 240, RGB_SURFACE);
    text(400, 170, 31, OPT_CENTER, RGB_TEXT, tr("STOP CHARGING?", "确定停止充电?"));
    if (s_ui.english) snprintf(b, sizeof(b), "Current cost CNY %lu.%02lu will be settled", (unsigned long)(s_ui.in.cost_cents / 100U), (unsigned long)(s_ui.in.cost_cents % 100U));
    else snprintf(b, sizeof(b), "当前费用 ¥%lu.%02lu 将结算", (unsigned long)(s_ui.in.cost_cents / 100U), (unsigned long)(s_ui.in.cost_cents % 100U));
    text(400, 220, 26, OPT_CENTER, RGB_DIM, b);
    outline_button(TAG_MODAL_CANCEL, 190, 260, 200, 80, RGB_LINE, tr("CANCEL", "取消"));
    button(TAG_MODAL_STOP, 410, 260, 200, 80, RGB_RED, RGB_TEXT, tr("STOP", "停止"));
}

static void draw(void)
{
    EVE_CoCmd_dlStart(&s_hal);
    /* Font bitmap state belongs to each display list, not just startup. */
    if (!s_ui.english && s_ui.fonts_ready)
        EVE_CoCmd_setFont2(&s_hal, FONT32_HANDLE, FONT32_RAMG, 0);
    EVE_CoDl_clearColorRgb(&s_hal, 0x10, 0x14, 0x18);
    EVE_CoDl_clear(&s_hal, 1, 1, 1);
    topbar();
    if (!s_ui.stop_modal) {
        EVE_CoDl_colorA(&s_hal, (uint8_t)((s_ui.now - s_ui.entered) < 150U ? ((s_ui.now - s_ui.entered) * 255U / 150U) : 255U));
        switch (s_ui.page) {
        case EV_PAGE_IDLE: page_idle(); break;
        case EV_PAGE_CONNECTED: page_connected(); break;
        case EV_PAGE_CHARGING: page_charging(); break;
        case EV_PAGE_SUMMARY: page_summary(); break;
        case EV_PAGE_FAULT: page_fault(); break;
        case EV_PAGE_SETTINGS: page_settings(); break;
        default: break;
        }
    } else {
        /* Do not spend external-flash bandwidth on content hidden by the dialog. */
        rect(0, 70, 800, 410, RGB_BASE);
    }
    EVE_CoDl_colorA(&s_hal, 255);
    if (s_ui.stop_modal) stop_modal();
    /* CMD_SWAP does not terminate RAM_DL. End every frame explicitly so
       scanout cannot execute stale commands beyond this frame's list. */
    EVE_CoDl_display(&s_hal);
    EVE_CoCmd_swap(&s_hal);
    EVE_Cmd_waitFlush(&s_hal);
}

static void enter_page(EvChargerPage page)
{
    s_ui.page = page; s_ui.entered = s_ui.now; s_ui.stop_modal = false;
    s_ui.stable_tag = 0; s_ui.raw_tag = 0; s_ui.released_tag = 0;
    if (page == EV_PAGE_CHARGING) s_ui.stop_sent = false;
}

static void sample_touch(void)
{
    uint8_t tag = s_ui.in.touch_allowed ? EVE_Hal_rd8(&s_hal, REG_TOUCH_TAG) : 0;
    s_ui.released_tag = 0;
    if (tag != s_ui.raw_tag) { s_ui.raw_tag = tag; s_ui.last_touch_change = s_ui.now; if (tag) s_ui.last_activity = s_ui.now; }
    if (tag && !s_ui.stable_tag && s_ui.now - s_ui.last_touch_change >= TOUCH_DOWN_MS) {
        s_ui.stable_tag = tag; s_ui.touch_started = s_ui.now; EVCharger_Beep(30);
    }
    if (!tag && s_ui.stable_tag && s_ui.now - s_ui.last_touch_change >= TOUCH_UP_MS) {
        s_ui.released_tag = s_ui.stable_tag; s_ui.stable_tag = 0;
    }
}

static void update(void)
{
    uint32_t page_age = s_ui.now - s_ui.entered;
    EVCharger_ReadInputs(&s_ui.in);
    if (!s_ui.filtered_lux) s_ui.filtered_lux = (uint32_t)s_ui.in.ambient_lux << 8;
    else s_ui.filtered_lux = (s_ui.filtered_lux * 149U + ((uint32_t)s_ui.in.ambient_lux << 8)) / 150U;
    if (s_ui.in.plug_connected != s_ui.previous_plug) {
        s_ui.previous_plug = s_ui.in.plug_connected;
        s_ui.last_activity = s_ui.now;
    }
    if (!s_ui.in.plug_connected) s_ui.ignore_plug_until_removed = false;
    sample_touch();
    if (s_ui.in.fault && s_ui.page != EV_PAGE_FAULT) { s_ui.page_before_fault = s_ui.page; enter_page(EV_PAGE_FAULT); EVCharger_Beep(90); }
    if (s_ui.page == EV_PAGE_FAULT && s_ui.in.retry_succeeded) { enter_page(s_ui.page_before_fault); return; }
    if (page_age < PAGE_LOCK_MS) return;
    if (s_ui.stable_tag == TAG_STOP && !s_ui.stop_sent && s_ui.now - s_ui.touch_started >= STOP_HOLD_MS) {
        EVCharger_RequestStop(); s_ui.stop_sent = true; s_ui.stop_modal = false; return;
    }
    if (s_ui.stable_tag == TAG_SETTINGS && s_ui.now - s_ui.touch_started >= SETTINGS_HOLD_MS) {
        if (s_ui.page == EV_PAGE_SETTINGS) enter_page(s_ui.page_before_settings);
        else { s_ui.page_before_settings = s_ui.page; enter_page(EV_PAGE_SETTINGS); s_ui.settings_unlocked = false; s_ui.pin_length = 0; }
        return;
    }
    if (s_ui.page == EV_PAGE_SETTINGS && !s_ui.settings_unlocked && s_ui.released_tag >= 20U && s_ui.released_tag <= 31U) {
        uint8_t key = (uint8_t)(s_ui.released_tag - 20U);
        if (key <= 8U && s_ui.pin_length < 4U) s_ui.pin[s_ui.pin_length++] = (char)('1' + key);
        else if (key == 10U && s_ui.pin_length < 4U) s_ui.pin[s_ui.pin_length++] = '0';
        else if (key == 9U) s_ui.pin_length = 0;
        else if (key == 11U) {
            s_ui.pin[s_ui.pin_length] = 0;
            if (!strcmp(s_ui.pin, "1234")) s_ui.settings_unlocked = true;
            s_ui.pin_length = 0;
        }
        return;
    }
    if (s_ui.released_tag == TAG_LANGUAGE) s_ui.english = !s_ui.english;
    if (s_ui.page == EV_PAGE_IDLE && s_ui.released_tag == TAG_WELCOME) {
#if defined(EV_CHARGER_DEMO)
        s_demo_started = true;
        s_demo_started_at = s_ui.now;
        s_demo_stopped = false;
        s_demo_meter_age = 5U;
#endif
        s_ui.ignore_plug_until_removed = false;
        enter_page(EV_PAGE_CONNECTED);
    }
    else if (s_ui.page == EV_PAGE_CONNECTED && s_ui.in.authorized) enter_page(EV_PAGE_CHARGING);
    else if (s_ui.page == EV_PAGE_CONNECTED && (s_ui.released_tag == TAG_CANCEL || page_age >= 60000U)) {
        EVCharger_RequestCancelAuthorization(); s_ui.ignore_plug_until_removed = true; enter_page(EV_PAGE_IDLE);
    }
    else if (s_ui.page == EV_PAGE_CHARGING && s_ui.in.charge_complete) enter_page(EV_PAGE_SUMMARY);
    else if (s_ui.page == EV_PAGE_CHARGING && !s_ui.stop_sent && s_ui.released_tag == TAG_STOP) { s_ui.stop_modal = true; s_ui.entered = s_ui.now; }
    else if (s_ui.stop_modal && page_age >= MODAL_LOCK_MS && s_ui.released_tag == TAG_MODAL_CANCEL) s_ui.stop_modal = false;
    else if (s_ui.stop_modal && page_age >= MODAL_LOCK_MS && s_ui.released_tag == TAG_MODAL_STOP) {
        EVCharger_RequestStop(); s_ui.stop_sent = true; s_ui.stop_modal = false;
    }
    else if (s_ui.page == EV_PAGE_SUMMARY && (s_ui.released_tag == TAG_DONE || !s_ui.in.plug_connected || page_age >= 30000U)) {
        s_ui.ignore_plug_until_removed = s_ui.in.plug_connected; enter_page(EV_PAGE_IDLE);
    }
    else if (s_ui.page == EV_PAGE_FAULT && s_ui.in.fault_retryable && s_ui.released_tag == TAG_RETRY) EVCharger_RequestRetry();
    else if (s_ui.page == EV_PAGE_SETTINGS && page_age >= 120000U) enter_page(s_ui.page_before_settings);
}

static uint8_t brightness_for_lux(uint16_t lux)
{
    if (lux <= 50U) return 38U; /* 30% of REG_PWM_DUTY full scale (128). */
    if (lux >= 5000U) return 128U;
    return (uint8_t)(38U + ((uint32_t)(lux - 50U) * 90U) / 4950U);
}

static uint8_t current_brightness(void)
{
    if (s_ui.page == EV_PAGE_FAULT) return 128U;
    if (s_ui.page == EV_PAGE_IDLE && !s_ui.in.plug_connected && s_ui.now - s_ui.last_activity >= 300000U) return 19U;
    return brightness_for_lux((uint16_t)(s_ui.filtered_lux >> 8));
}

static const char *page_name(void)
{
    static const char *const names[] = { "idle", "connected", "charging", "summary", "fault", "settings" };
    return s_ui.stop_modal ? "charging/stop-dialog" : names[s_ui.page];
}

static void monitor_underrun(void)
{
#if EVE_SUPPORT_GEN >= EVE4 && !defined(BT8XXEMU_PLATFORM)
    if (s_ui.now - s_ui.last_underrun_check >= 1000U) {
        uint32_t count = EVE_Hal_rd32(&s_hal, REG_UNDERRUN);
        if (count != s_ui.last_underrun) {
            printf("EVCharger underrun page=%s lang=%s total=%lu delta=%lu\n", page_name(),
                s_ui.english ? "en" : "zh", (unsigned long)count,
                (unsigned long)(count - s_ui.last_underrun));
            s_ui.last_underrun = count;
        }
        s_ui.last_underrun_check = s_ui.now;
    }
#endif
}

static void update_backlight(void)
{
    uint8_t requested = current_brightness();
    uint8_t delta = requested > s_ui.pwm_duty ? requested - s_ui.pwm_duty : s_ui.pwm_duty - requested;
    if (!s_ui.pwm_duty || delta >= 2U || s_ui.page == EV_PAGE_FAULT) {
        if (requested != s_ui.pwm_duty) {
            EVE_Hal_wr8(&s_hal, REG_PWM_DUTY, requested);
            s_ui.pwm_duty = requested;
        }
    }
}

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;
    memset(&s_ui, 0, sizeof(s_ui)); s_ui.english = true; s_ui.page = EV_PAGE_IDLE;
    Gpu_Init(&s_hal);
    log_display_config();
    load_flash_fonts();
#if !defined(BT8XXEMU_PLATFORM)
    Esd_Calibrate(&s_hal);
#endif
    s_ui.entered = EVE_millis();
    s_ui.last_activity = s_ui.entered;
    /* Continuous list rebuild; see docs/best-practices/display-list-update-strategies.md. */
    while (TRUE) {
        s_ui.now = EVE_millis();
        update();
        update_backlight();
        draw();
        monitor_underrun();
        EVE_sleep(33);
    }
    return 0;
}
