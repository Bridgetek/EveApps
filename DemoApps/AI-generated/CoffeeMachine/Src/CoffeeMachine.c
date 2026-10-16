/**
 * @file CoffeeMachine.c
 * @brief BT817/RP2040 1280x800 bilingual coffee-machine HMI demonstration.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Common.h"
#include "EVE_CoCmd.h"
#include "CoffeeMachine.h"
#include "CoffeeMachineAssets.h"
#include "CoffeeMachineController.h"
#include "CoffeeMachinePlatform.h"
#include "CoffeeMachineStrings.h"
#include "Maths.h"

#define RGB_BG       0x17110DUL
#define RGB_PANEL    0x2A1F18UL
#define RGB_GLOW     0x4D3828UL
#define RGB_CONTROL  0x3A2B21UL
#define RGB_TEXT     0xF3E6D3UL
#define RGB_MUTED    0xB9A58FUL
#define RGB_ACCENT   0xE0902FUL
#define RGB_ON_ACCENT 0x1A0F08UL
#define RGB_STOP     0xE5675AUL
#define RGB_ERROR    0xB83D34UL

#define TOUCH_DEBOUNCE_MS 30U
#define HOME_IDLE_MS 60000U
#define STATIC_STATUS_MS 250U
#define DONE_HOLD_MS 3000U
#define DL_BUDGET_BYTES 8192UL
#define UNDERRUN_REPORT_MS 1000U
#define PROBE_REPLAY_MS 5000U
#define TAG_WAKE 30U

typedef struct DrinkConfig {
    uint16_t volume;
    uint16_t minimum;
    uint16_t maximum;
    uint8_t milk;
} DrinkConfig;

typedef struct ScanoutProbe {
    uint32_t frames;
    uint32_t underrun;
    bool valid;
} ScanoutProbe;

typedef struct CoffeeUi {
    CoffeePage page;
    CoffeePage previousPage;
    CoffeeLanguage language;
    CoffeeDrinkId drink;
    CoffeeStatus status;
    uint16_t volume;
    uint8_t strength;
    uint8_t temperature;
    uint8_t milkRatio;
    uint8_t rawTag;
    uint8_t pressedTag;
    uint8_t releasedTag;
    uint16_t touchX;
    bool rawTouching;
    bool touchCanceled;
    uint32_t rawChanged;
    uint32_t now;
    uint32_t lastActivity;
    uint32_t lastDraw;
    uint32_t lastStatus;
    uint32_t doneAt;
    uint32_t lastUnderrunCheck;
    uint32_t lastUnderrun;
    uint32_t lastFrames;
    uint32_t lastProbeReplay;
    bool languageSelected;
    bool dirty;
} CoffeeUi;

static const DrinkConfig s_drinks[COFFEE_DRINK_COUNT] = {
    { 40, 25, 80, 0 }, { 110, 70, 160, 0 }, { 180, 120, 260, 0 }, { 180, 120, 260, 1 },
    { 240, 160, 340, 1 }, { 160, 120, 240, 1 }, { 200, 100, 350, 0 }, { 120, 80, 220, 1 }
};

static EVE_HalContext s_hal;
static CoffeeUi s_ui;
static ScanoutProbe s_probe[9]; /* Solid, Standby, Home, Customize, Brewing without/with ring. */
static bool s_probeHideDrinkImages;
static bool s_probeHideBrewGauge;

static uint8_t pageCacheFont(CoffeePage page)
{
    if (page == COFFEE_PAGE_LANGUAGE || page == COFFEE_PAGE_STANDBY)
        return COFFEE_FONT44_HANDLE;
    return COFFEE_FONT32_HANDLE;
}

static const char *pageName(CoffeePage page)
{
    switch (page) {
    case COFFEE_PAGE_HOME: return "home";
    case COFFEE_PAGE_CUSTOMIZE: return "customize";
    case COFFEE_PAGE_BREWING: return "brewing";
    case COFFEE_PAGE_LANGUAGE: return "language";
    case COFFEE_PAGE_STANDBY: return "standby";
    case COFFEE_PAGE_QUICK_CONFIRM: return "quick-confirm";
    default: return "unknown";
    }
}

static void monitorUnderrun(void)
{
    uint32_t count, frames;
    if ((uint32_t)(s_ui.now - s_ui.lastUnderrunCheck) < UNDERRUN_REPORT_MS)
        return;
    count = EVE_Hal_rd32(&s_hal, REG_UNDERRUN);
    frames = EVE_Hal_rd32(&s_hal, REG_FRAMES);
    printf("CoffeeMachine scanout page=%s lang=%s frames=%lu underrun=%lu total=%lu cache=%u\n",
        pageName(s_ui.page),
        s_ui.language == COFFEE_LANGUAGE_CHINESE ? "zh" : "en",
        (unsigned long)(frames - s_ui.lastFrames),
        (unsigned long)(count - s_ui.lastUnderrun),
        (unsigned long)count, (unsigned)pageCacheFont(s_ui.page));
    s_ui.lastUnderrunCheck = s_ui.now;
    s_ui.lastUnderrun = count;
    s_ui.lastFrames = frames;
    if ((uint32_t)(s_ui.now - s_ui.lastProbeReplay) >= PROBE_REPLAY_MS) {
        if (s_probe[0].valid && s_probe[1].valid && s_probe[2].valid &&
            s_probe[3].valid && s_probe[4].valid &&
            s_probe[5].valid && s_probe[6].valid &&
            s_probe[7].valid && s_probe[8].valid) {
            printf("CoffeeMachine probes frames/underrun solid=%lu/%lu standby44=%lu/%lu standby44+hint=%lu/%lu\n",
                (unsigned long)s_probe[0].frames, (unsigned long)s_probe[0].underrun,
                (unsigned long)s_probe[1].frames, (unsigned long)s_probe[1].underrun,
                (unsigned long)s_probe[2].frames, (unsigned long)s_probe[2].underrun);
            printf("CoffeeMachine probes frames/underrun home-no-images=%lu/%lu home-images=%lu/%lu hcycle=%lu vcycle=%lu pclk=%lu frequency=%lu\n",
                (unsigned long)s_probe[3].frames, (unsigned long)s_probe[3].underrun,
                (unsigned long)s_probe[4].frames, (unsigned long)s_probe[4].underrun,
                (unsigned long)EVE_Hal_rd32(&s_hal, REG_HCYCLE),
                (unsigned long)EVE_Hal_rd32(&s_hal, REG_VCYCLE),
                (unsigned long)EVE_Hal_rd32(&s_hal, REG_PCLK),
                (unsigned long)EVE_Hal_rd32(&s_hal, REG_FREQUENCY));
            printf("CoffeeMachine probes frames/underrun customize-no-image=%lu/%lu customize-image=%lu/%lu\n",
                (unsigned long)s_probe[5].frames, (unsigned long)s_probe[5].underrun,
                (unsigned long)s_probe[6].frames, (unsigned long)s_probe[6].underrun);
            printf("CoffeeMachine probes frames/underrun brewing-no-ring=%lu/%lu brewing-ring=%lu/%lu\n",
                (unsigned long)s_probe[7].frames, (unsigned long)s_probe[7].underrun,
                (unsigned long)s_probe[8].frames, (unsigned long)s_probe[8].underrun);
        }
        else
            printf("CoffeeMachine probes incomplete; check startup fault log\n");
        s_ui.lastProbeReplay = s_ui.now;
    }
}

static void color(uint32_t value)
{
    EVE_CoDl_colorRgb(&s_hal, (uint8_t)(value >> 16),
        (uint8_t)(value >> 8), (uint8_t)value);
}

static uint32_t pressedColor(uint8_t tag, uint32_t value)
{
    uint32_t r, g, b;
    if (s_ui.pressedTag != tag) return value;
    r = (value >> 16) & 255U; g = (value >> 8) & 255U; b = value & 255U;
    r += (255U - r) / 6U; g += (255U - g) / 6U; b += (255U - b) / 6U;
    return (r << 16) | (g << 8) | b;
}

static void roundRect(int16_t x, int16_t y, int16_t w, int16_t h,
    int16_t radius, uint32_t fill)
{
    if (radius < 1) radius = 1;
    color(fill);
    EVE_CoDl_lineWidth(&s_hal, (uint16_t)(radius * 16));
    EVE_CoDl_begin(&s_hal, RECTS);
    EVE_CoDl_vertex2f(&s_hal, x + radius, y + radius);
    EVE_CoDl_vertex2f(&s_hal, x + w - radius, y + h - radius);
    EVE_CoDl_end(&s_hal);
}

static uint8_t selectTextFont(CoffeePage page, uint16_t pixels,
    const char *text)
{
    const unsigned char *p = (const unsigned char *)text;
    bool ascii = true;
    uint8_t font = CoffeeAssets_Font(pixels);
    for (; *p; ++p) {
        if (*p & 0x80U) { ascii = false; break; }
    }
    /* Each page must use at most its one cached extended font. ASCII labels
     * use BT817 ROM fonts instead of competing external-flash glyphs. */
    if (page == COFFEE_PAGE_LANGUAGE)
        font = ascii ? (pixels <= 24U ? 28U : (pixels <= 32U ? 29U : 31U)) :
            COFFEE_FONT44_HANDLE;
    else if ((page == COFFEE_PAGE_HOME || page == COFFEE_PAGE_QUICK_CONFIRM) && pixels != 32U)
        font = ascii ? (pixels <= 24U ? 28U : 29U) : COFFEE_FONT32_HANDLE;
    else if (page == COFFEE_PAGE_CUSTOMIZE) {
        if (pixels == 44U)
            font = ascii ? 31U : COFFEE_FONT32_HANDLE;
        else if (pixels != 32U)
            font = ascii ? (pixels <= 24U ? 28U : 29U) : COFFEE_FONT32_HANDLE;
    }
    else if (page == COFFEE_PAGE_BREWING)
        font = ascii ? (pixels <= 24U ? 28U : (pixels <= 32U ? 29U : 31U)) :
            COFFEE_FONT32_HANDLE;
    else if (page == COFFEE_PAGE_STANDBY && pixels != 44U)
        font = ascii ? 29U : COFFEE_FONT44_HANDLE;
    return font;
}

static void textAt(int16_t x, int16_t y, uint16_t pixels,
    uint16_t options, uint32_t value, const char *text)
{
    uint8_t font = selectTextFont(s_ui.page, pixels, text);
    color(value);
    EVE_CoCmd_text(&s_hal, x, y, font, options, text);
}

static const char *tr(CoffeeStringId id)
{
    return CoffeeStrings_Get(s_ui.language, id);
}

static void taggedButton(uint8_t tag, int16_t x, int16_t y, int16_t w,
    int16_t h, uint32_t fill, uint32_t foreground, const char *label)
{
    EVE_CoDl_tagMask(&s_hal, 1);
    EVE_CoDl_tag(&s_hal, tag);
    roundRect(x, y, w, h, 20, pressedColor(tag, fill));
    textAt((int16_t)(x + w / 2), (int16_t)(y + h / 2), 32,
        OPT_CENTER, foreground, label);
    EVE_CoDl_tag(&s_hal, 0);
    EVE_CoDl_tagMask(&s_hal, 0);
}

static void statusPill(int16_t x, int16_t w, const char *label)
{
    roundRect(x, 12, w, 48, 24, RGB_PANEL);
    textAt((int16_t)(x + w / 2), 36, 24, OPT_CENTER, RGB_MUTED, label);
}

static void temperaturePill(int16_t x, int16_t w, int16_t celsius)
{
    int16_t center = (int16_t)(x + w / 2);
    roundRect(x, 12, w, 48, 24, RGB_PANEL);
    EVE_CoDl_saveContext(&s_hal);
    color(RGB_MUTED);
    EVE_CoCmd_number(&s_hal, (int16_t)(center - 2), 36, 28,
        OPT_RIGHTX | OPT_CENTERY, celsius);
    EVE_CoDl_pointSize(&s_hal, 5U * 16U);
    EVE_CoDl_begin(&s_hal, POINTS);
    EVE_CoDl_vertex2f(&s_hal, (int16_t)(center + 4), 28);
    EVE_CoDl_end(&s_hal);
    color(RGB_PANEL);
    EVE_CoDl_pointSize(&s_hal, 3U * 16U);
    EVE_CoDl_begin(&s_hal, POINTS);
    EVE_CoDl_vertex2f(&s_hal, (int16_t)(center + 4), 28);
    EVE_CoDl_end(&s_hal);
    color(RGB_MUTED);
    EVE_CoCmd_text(&s_hal, (int16_t)(center + 11), 36, 28,
        OPT_CENTERY, "C");
    EVE_CoDl_restoreContext(&s_hal);
}

static void drawStatusBar(void)
{
    char buffer[32];
    const char *state = s_ui.status.state == COFFEE_CONTROLLER_BUSY ? tr(COFFEE_STR_BUSY) :
        (s_ui.status.state == COFFEE_CONTROLLER_ERROR ? tr(COFFEE_STR_ERROR) : tr(COFFEE_STR_READY));
    roundRect(0, 0, 1280, 72, 0, RGB_BG);
    textAt(24, 36, 28, OPT_CENTERY, RGB_TEXT, "10:28");
    statusPill(126, 112, state);
    temperaturePill(254, 118, s_ui.status.boilerCelsius);
    snprintf(buffer, sizeof(buffer), "%s %u%%", tr(COFFEE_STR_WATER),
        s_ui.status.waterPercent);
    statusPill(388, 154, buffer);
    snprintf(buffer, sizeof(buffer), "%s %u%%", tr(COFFEE_STR_BEANS),
        s_ui.status.beansPercent);
    statusPill(558, 210, buffer);
    snprintf(buffer, sizeof(buffer), "%s %u%%", tr(COFFEE_STR_GROUNDS),
        s_ui.status.groundsPercent);
    statusPill(784, 210, buffer);
    taggedButton(COFFEE_TAG_LANGUAGE, 1148, 12, 108, 48, RGB_ACCENT,
        RGB_ON_ACCENT, s_ui.language == COFFEE_LANGUAGE_CHINESE ? "中文" : "EN");
}

static void drawAlert(void)
{
    if (s_ui.status.heartbeatOk) return;
    roundRect(24, 78, 1232, 54, 16, RGB_ERROR);
    textAt(640, 105, 28, OPT_CENTER, RGB_TEXT, tr(COFFEE_STR_HEARTBEAT_LOST));
}

static void drawDrinkTile(uint8_t index, int16_t x, int16_t y)
{
    char volume[24];
    uint8_t tag = (uint8_t)(COFFEE_TAG_DRINK_FIRST + index);
    int16_t offset = s_ui.pressedTag == tag ? 3 : 0;
    EVE_CoDl_tagMask(&s_hal, 1); EVE_CoDl_tag(&s_hal, tag);
    roundRect((int16_t)(x + offset), (int16_t)(y + offset), 290, 280, 28,
        pressedColor(tag, RGB_PANEL));
    roundRect((int16_t)(x + 12 + offset), (int16_t)(y + 12 + offset), 266, 180, 22, RGB_GLOW);
    if (!s_probeHideDrinkImages)
        CoffeeAssets_DrawDrink(&s_hal, (CoffeeDrinkId)index,
            (int16_t)(x + 35 + offset), (int16_t)(y + 12 + offset), 220, 165, false);
    textAt((int16_t)(x + 145 + offset), (int16_t)(y + 216 + offset), 32,
        OPT_CENTER, RGB_TEXT, CoffeeStrings_Drink(s_ui.language, (CoffeeDrinkId)index));
    snprintf(volume, sizeof(volume), "%u ml", s_drinks[index].volume);
    textAt((int16_t)(x + 145 + offset), (int16_t)(y + 252 + offset), 24,
        OPT_CENTER, RGB_MUTED, volume);
    EVE_CoDl_tag(&s_hal, 0); EVE_CoDl_tagMask(&s_hal, 0);
}

static void drawHome(void)
{
    uint8_t i;
    drawStatusBar();
    for (i = 0; i < COFFEE_DRINK_COUNT; ++i)
        drawDrinkTile(i, (int16_t)(24 + (i % 4U) * 314),
            (int16_t)(84 + (i / 4U) * 292));
    roundRect(0, 680, 1280, 120, 0, RGB_PANEL);
    taggedButton(COFFEE_TAG_CLEAN, 24, 696, 190, 88, RGB_CONTROL, RGB_TEXT, tr(COFFEE_STR_CLEANING));
    taggedButton(COFFEE_TAG_HOT_WATER, 238, 696, 190, 88, RGB_CONTROL, RGB_TEXT, tr(COFFEE_STR_HOT_WATER));
    textAt(640, 740, 28, OPT_CENTER, RGB_MUTED, tr(COFFEE_STR_HOME_HINT));
    taggedButton(COFFEE_TAG_STANDBY, 1066, 696, 190, 88, RGB_CONTROL, RGB_TEXT, tr(COFFEE_STR_STANDBY));
    drawAlert();
}

static void drawSegments(uint8_t firstTag, uint8_t count, uint8_t selected,
    int16_t x, int16_t y, int16_t width, const char *const *labels)
{
    uint8_t i;
    int16_t segment = (int16_t)((width - (count - 1U) * 12U) / count);
    for (i = 0; i < count; ++i) {
        char number[4];
        const char *label = labels ? labels[i] : number;
        if (!labels) snprintf(number, sizeof(number), "%u", (unsigned)(i + 1U));
        taggedButton((uint8_t)(firstTag + i), (int16_t)(x + i * (segment + 12)), y,
            segment, 76, i == selected ? RGB_ACCENT : RGB_CONTROL,
            i == selected ? RGB_ON_ACCENT : RGB_TEXT, label);
    }
}

static void drawSlider(uint8_t tag, int16_t x, int16_t y, int16_t width,
    uint16_t value, uint16_t minimum, uint16_t maximum, const char *label, const char *suffix)
{
    char shown[32];
    int16_t filled = (int16_t)((uint32_t)(value - minimum) * width / (maximum - minimum));
    textAt(x, y, 28, 0, RGB_MUTED, label);
    snprintf(shown, sizeof(shown), "%u%s", value, suffix);
    textAt((int16_t)(x + width), y, 28, OPT_RIGHTX, RGB_TEXT, shown);
    EVE_CoDl_tagMask(&s_hal, 1); EVE_CoDl_tag(&s_hal, tag);
    /* Make the full drag corridor touchable without changing its appearance. */
    EVE_CoDl_colorMask(&s_hal, false, false, false, false);
    EVE_CoDl_begin(&s_hal, RECTS);
    EVE_CoDl_vertex2f(&s_hal, (int16_t)(x - 12), (int16_t)(y + 24));
    EVE_CoDl_vertex2f(&s_hal, (int16_t)(x + width + 12), (int16_t)(y + 84));
    EVE_CoDl_end(&s_hal);
    EVE_CoDl_colorMask(&s_hal, true, true, true, true);
    roundRect(x, (int16_t)(y + 48), width, 12, 6, RGB_CONTROL);
    if (filled > 0) roundRect(x, (int16_t)(y + 48), filled, 12, 6, RGB_ACCENT);
    color(s_ui.pressedTag == tag ? RGB_TEXT : RGB_ACCENT);
    EVE_CoDl_pointSize(&s_hal, 24U * 16U); EVE_CoDl_begin(&s_hal, POINTS);
    EVE_CoDl_vertex2f(&s_hal, (int16_t)(x + filled), (int16_t)(y + 54)); EVE_CoDl_end(&s_hal);
    EVE_CoDl_tag(&s_hal, 0); EVE_CoDl_tagMask(&s_hal, 0);
}

static void drawCustomize(void)
{
    static const char *const temperatureEn[] = { "Warm", "Hot", "Extra hot" };
    static const char *const temperatureZh[] = { "温", "热", "烫" };
    const char *const *temperature = s_ui.language == COFFEE_LANGUAGE_CHINESE ? temperatureZh : temperatureEn;
    char summary[64];
    drawStatusBar();
    roundRect(24, 84, 420, 572, 28, RGB_PANEL);
    roundRect(48, 108, 372, 322, 24, RGB_GLOW);
    if (!s_probeHideDrinkImages)
        CoffeeAssets_DrawDrink(&s_hal, s_ui.drink, 52, 130, 364, 273, false);
    textAt(234, 478, 44, OPT_CENTER, RGB_TEXT, CoffeeStrings_Drink(s_ui.language, s_ui.drink));
    snprintf(summary, sizeof(summary), "%u ml / %u / %s", s_ui.volume, s_ui.strength,
        temperature[s_ui.temperature]);
    textAt(234, 526, 24, OPT_CENTER, RGB_MUTED, summary);
    textAt(492, 108, 28, 0, RGB_MUTED, tr(COFFEE_STR_STRENGTH));
    drawSegments(COFFEE_TAG_STRENGTH_FIRST, 5, (uint8_t)(s_ui.strength - 1U), 492, 144, 740, NULL);
    drawSlider(COFFEE_TAG_SIZE, 492, 250, 740, s_ui.volume,
        s_drinks[s_ui.drink].minimum, s_drinks[s_ui.drink].maximum, tr(COFFEE_STR_SIZE), " ml");
    if (s_drinks[s_ui.drink].milk)
        drawSlider(COFFEE_TAG_MILK, 492, 352, 740, s_ui.milkRatio, 0, 100, tr(COFFEE_STR_MILK_RATIO), "%");
    textAt(492, 466, 28, 0, RGB_MUTED, tr(COFFEE_STR_TEMPERATURE));
    drawSegments(COFFEE_TAG_TEMP_FIRST, 3, s_ui.temperature, 492, 502, 740, temperature);
    roundRect(0, 680, 1280, 120, 0, RGB_PANEL);
    taggedButton(COFFEE_TAG_BACK, 24, 696, 220, 88, RGB_CONTROL, RGB_TEXT, tr(COFFEE_STR_BACK));
    if (s_ui.status.heartbeatOk)
        taggedButton(COFFEE_TAG_START, 836, 696, 420, 88, RGB_ACCENT, RGB_ON_ACCENT, tr(COFFEE_STR_START));
    else {
        roundRect(836, 696, 420, 88, 20, RGB_CONTROL);
        textAt(1046, 740, 32, OPT_CENTER, RGB_MUTED, tr(COFFEE_STR_START));
    }
    drawAlert();
}

static void drawQuickConfirm(void)
{
    char volume[24];
    drawStatusBar();
    roundRect(0, 84, 1280, 572, 0, RGB_BG);
    roundRect(250, 160, 780, 400, 28, RGB_PANEL);
    textAt(640, 300, 32, OPT_CENTER, RGB_TEXT,
        CoffeeStrings_Drink(s_ui.language, s_ui.drink));
    snprintf(volume, sizeof(volume), "%u ml", s_ui.volume);
    textAt(640, 380, 32, OPT_CENTER, RGB_MUTED, volume);
    roundRect(0, 680, 1280, 120, 0, RGB_PANEL);
    taggedButton(COFFEE_TAG_BACK, 24, 696, 220, 88,
        RGB_CONTROL, RGB_TEXT, tr(COFFEE_STR_BACK));
    if (s_ui.status.heartbeatOk)
        taggedButton(COFFEE_TAG_START, 836, 696, 420, 88,
            RGB_ACCENT, RGB_ON_ACCENT, tr(COFFEE_STR_START));
    else {
        roundRect(836, 696, 420, 88, 20, RGB_CONTROL);
        textAt(1046, 740, 32, OPT_CENTER, RGB_MUTED, tr(COFFEE_STR_START));
    }
    drawAlert();
}

static CoffeeStringId stepString(CoffeeBrewStep step, CoffeeDrinkId drink)
{
    if (step == COFFEE_STEP_GRINDING) return COFFEE_STR_GRINDING;
    if (step == COFFEE_STEP_MILK)
        return s_drinks[drink].milk ? COFFEE_STR_MILK : COFFEE_STR_BREWING;
    if (step == COFFEE_STEP_DONE) return COFFEE_STR_DONE;
    return COFFEE_STR_BREWING;
}

static void ringPoint(int16_t cx, int16_t cy, uint16_t radius, uint16_t angle)
{
    int16_t x = (int16_t)(cx + ((int32_t)radius * Math_Qsin(angle)) / 32767L);
    int16_t y = (int16_t)(cy - ((int32_t)radius * Math_Qcos(angle)) / 32767L);
    EVE_CoDl_vertex2f(&s_hal, x, y);
}

static void drawProgressRing(uint8_t progress)
{
    const int16_t cx = 640;
    const int16_t cy = 338;
    const uint16_t radius = 154U;
    const uint16_t stroke = 22U;
    const uint16_t segments = 72U;
    uint16_t i;
    uint16_t activeSegments;
    uint32_t activeAngle;
    if (progress > 100U) progress = 100U;
    activeSegments = (uint16_t)(((uint32_t)segments * progress + 99U) / 100U);
    activeAngle = ((uint32_t)65536U * progress) / 100U;

    color(RGB_CONTROL);
    EVE_CoDl_lineWidth(&s_hal, (uint16_t)(stroke * 16U));
    EVE_CoDl_begin(&s_hal, LINE_STRIP);
    for (i = 0; i <= segments; ++i)
        ringPoint(cx, cy, radius, (uint16_t)(((uint32_t)65536U * i) / segments));
    EVE_CoDl_end(&s_hal);

    if (activeSegments == 0U) return;
    color(RGB_ACCENT);
    EVE_CoDl_lineWidth(&s_hal, (uint16_t)(stroke * 16U));
    EVE_CoDl_begin(&s_hal, LINE_STRIP);
    for (i = 0; i <= activeSegments; ++i)
        ringPoint(cx, cy, radius, (uint16_t)((activeAngle * i) / activeSegments));
    EVE_CoDl_end(&s_hal);

    /* EVE line-strip joins are sharp; endpoint dots give the progress arc
     * the rounded caps used in the reference design. */
    EVE_CoDl_pointSize(&s_hal, (uint16_t)(stroke * 16U));
    EVE_CoDl_begin(&s_hal, POINTS);
    ringPoint(cx, cy, radius, 0U);
    ringPoint(cx, cy, radius, (uint16_t)activeAngle);
    EVE_CoDl_end(&s_hal);
}

static uint16_t remainingSeconds(uint16_t total, uint16_t elapsed)
{
    return total > elapsed ? (uint16_t)(total - elapsed) : 0U;
}

static void drawBrewing(void)
{
    char elapsed[24];
    char total[24];
    uint16_t displaySeconds = remainingSeconds(s_ui.status.totalSeconds,
        s_ui.status.elapsedSeconds);
    bool quickDrink = s_ui.drink == COFFEE_DRINK_HOT_WATER ||
        s_ui.drink == COFFEE_DRINK_MILK_FOAM;
    drawStatusBar();
    if (!s_probeHideBrewGauge) drawProgressRing(s_ui.status.progress);
    snprintf(elapsed, sizeof(elapsed), "%02u:%02u",
        displaySeconds / 60U, displaySeconds % 60U);
    if (s_ui.language == COFFEE_LANGUAGE_CHINESE)
        snprintf(total, sizeof(total), "/ %02u:%02u",
            s_ui.status.totalSeconds / 60U, s_ui.status.totalSeconds % 60U);
    else
        snprintf(total, sizeof(total), "of %02u:%02u",
            s_ui.status.totalSeconds / 60U, s_ui.status.totalSeconds % 60U);
    /* Keep Brewing on one external-font cache. The timer uses ROM font 31;
     * every Chinese string is routed to the cached 32 px font. */
    textAt(640, 324, 44, OPT_CENTER, RGB_TEXT, elapsed);
    textAt(640, 394, 28, OPT_CENTER, RGB_MUTED, total);
    textAt(640, 532, 44, OPT_CENTER, RGB_TEXT,
        CoffeeStrings_Drink(s_ui.language, s_ui.drink));
    textAt(640, 584, 32, OPT_CENTER, RGB_ACCENT,
        quickDrink ? tr(COFFEE_STR_READY) :
        tr(stepString(s_ui.status.step, s_ui.drink)));
    roundRect(0, 680, 1280, 120, 0, RGB_PANEL);
    taggedButton(COFFEE_TAG_STOP, 430, 696, 420, 88, RGB_STOP, RGB_ON_ACCENT, tr(COFFEE_STR_STOP));
    drawAlert();
}

static void drawLanguageCard(uint8_t tag, int16_t x, CoffeeLanguage language,
    const char *title, const char *subtitle)
{
    bool selected = s_ui.language == language;
    EVE_CoDl_tagMask(&s_hal, 1); EVE_CoDl_tag(&s_hal, tag);
    roundRect(x, 190, 440, 320, 28, pressedColor(tag, RGB_PANEL));
    if (selected) {
        roundRect((int16_t)(x + 4), 194, 432, 4, 2, RGB_ACCENT);
        roundRect((int16_t)(x + 392), 210, 32, 32, 16, RGB_ACCENT);
        textAt((int16_t)(x + 408), 226, 24, OPT_CENTER, RGB_ON_ACCENT, "+");
    }
    textAt((int16_t)(x + 220), 320, 44, OPT_CENTER, RGB_TEXT, title);
    textAt((int16_t)(x + 220), 374, 28, OPT_CENTER, RGB_MUTED, subtitle);
    EVE_CoDl_tag(&s_hal, 0); EVE_CoDl_tagMask(&s_hal, 0);
}

static void drawLanguage(void)
{
    textAt(640, 88, 44, OPT_CENTER, RGB_TEXT, tr(COFFEE_STR_LANGUAGE_TITLE));
    drawLanguageCard(COFFEE_TAG_ENGLISH, 176, COFFEE_LANGUAGE_ENGLISH, "English", "English");
    drawLanguageCard(COFFEE_TAG_CHINESE, 664, COFFEE_LANGUAGE_CHINESE, "中文", "简体中文");
    roundRect(0, 680, 1280, 120, 0, RGB_PANEL);
    if (s_ui.languageSelected)
        taggedButton(COFFEE_TAG_BACK, 24, 696, 260, 88, RGB_CONTROL, RGB_TEXT, "Back / 返回");
    else {
        roundRect(24, 696, 260, 88, 20, RGB_CONTROL);
        textAt(154, 740, 28, OPT_CENTER, RGB_MUTED, "Back / 返回");
    }
}

static void drawStandby(void)
{
    EVE_CoDl_tagMask(&s_hal, 1); EVE_CoDl_tag(&s_hal, TAG_WAKE);
    roundRect(0, 0, 1280, 800, 0, 0x080604UL);
    textAt(640, 350, 44, OPT_CENTER, RGB_TEXT, tr(COFFEE_STR_STANDBY));
    textAt(640, 418, 28, OPT_CENTER, RGB_MUTED, tr(COFFEE_STR_STANDBY_HINT));
    EVE_CoDl_tag(&s_hal, 0); EVE_CoDl_tagMask(&s_hal, 0);
}

static bool drawFrame(void)
{
    uint32_t dlBytes;
    EVE_CoCmd_dlStart(&s_hal);
    CoffeeAssets_BindFonts(&s_hal);
    EVE_CoCmd_romFont(&s_hal, 28, 28);
    EVE_CoCmd_romFont(&s_hal, 29, 29);
    EVE_CoCmd_romFont(&s_hal, 31, 31);
    if (!CoffeeAssets_CacheFont(&s_hal, pageCacheFont(s_ui.page))) return false;
    EVE_CoDl_clearColorRgb(&s_hal, 0x17, 0x11, 0x0D);
    EVE_CoDl_clear(&s_hal, 1, 1, 1);
    EVE_CoDl_vertexFormat(&s_hal, 0);
    switch (s_ui.page) {
    case COFFEE_PAGE_HOME: drawHome(); break;
    case COFFEE_PAGE_CUSTOMIZE: drawCustomize(); break;
    case COFFEE_PAGE_QUICK_CONFIRM: drawQuickConfirm(); break;
    case COFFEE_PAGE_BREWING: drawBrewing(); break;
    case COFFEE_PAGE_LANGUAGE: drawLanguage(); break;
    case COFFEE_PAGE_STANDBY: drawStandby(); break;
    default: break;
    }
    EVE_CoDl_display(&s_hal);
    if (!EVE_Cmd_waitFlush(&s_hal)) {
        char report[129];
        EVE_Hal_rdMem(&s_hal, (uint8_t *)report, RAM_ERR_REPORT, 128);
        report[128] = 0;
        fprintf(stderr, "CoffeeMachine: display-list fault on %s: %s\n",
            pageName(s_ui.page), report);
        return false;
    }
    dlBytes = EVE_Hal_rd32(&s_hal, REG_CMD_DL);
    if (dlBytes > DL_BUDGET_BYTES) {
        fprintf(stderr, "CoffeeMachine: display list %lu exceeds %lu-byte budget\n",
            (unsigned long)dlBytes, (unsigned long)DL_BUDGET_BYTES);
        return false;
    }
    EVE_CoCmd_swap(&s_hal);
    if (!EVE_Cmd_waitFlush(&s_hal)) {
        char report[129];
        EVE_Hal_rdMem(&s_hal, (uint8_t *)report, RAM_ERR_REPORT, 128);
        report[128] = 0;
        fprintf(stderr, "CoffeeMachine: swap fault on %s: %s\n",
            pageName(s_ui.page), report);
        return false;
    }
    return true;
}

static void enterPage(CoffeePage page)
{
    s_ui.page = page;
    s_ui.pressedTag = 0;
    s_ui.rawTag = 0;
    s_ui.releasedTag = 0;
    s_ui.rawTouching = false;
    s_ui.touchCanceled = false;
    s_ui.dirty = true;
}

static void chooseDrink(CoffeeDrinkId drink)
{
    s_ui.drink = drink;
    s_ui.volume = s_drinks[drink].volume;
    s_ui.strength = 3;
    s_ui.temperature = 1;
    s_ui.milkRatio = s_drinks[drink].milk ? 50 : 0;
    enterPage(drink == COFFEE_DRINK_HOT_WATER || drink == COFFEE_DRINK_MILK_FOAM ?
        COFFEE_PAGE_QUICK_CONFIRM : COFFEE_PAGE_CUSTOMIZE);
}

static void startSelectedDrink(void)
{
    CoffeeBrewRequest request;
    CoffeeStatus started;
    if (!s_ui.status.heartbeatOk) return;
    request.drink = s_ui.drink;
    request.volumeMl = s_ui.volume;
    request.strength = s_ui.strength;
    request.temperature = s_ui.temperature;
    request.milkRatio = s_ui.milkRatio;
    CoffeeController_RequestBrew(&request);
    /* Replace the previous operation's final status before the brewing page
     * can draw. Otherwise its 100% ring is visible for one frame until the
     * normal 250 ms status poll reads the newly started brew. */
    memset(&s_ui.status, 0, sizeof(s_ui.status));
    s_ui.status.state = COFFEE_CONTROLLER_BUSY;
    s_ui.status.step = COFFEE_STEP_BREWING;
    CoffeeController_ReadStatus(&started);
    s_ui.status = started;
    s_ui.doneAt = 0;
    enterPage(COFFEE_PAGE_BREWING);
}

static void sampleTouch(void)
{
    uint32_t xy = EVE_Hal_rd32(&s_hal, REG_TOUCH_SCREEN_XY);
    bool touching = xy != 0x80008000UL;
    uint8_t tag = touching ? EVE_Hal_rd8(&s_hal, REG_TOUCH_TAG) : 0;
    bool sliderDrag = s_ui.page == COFFEE_PAGE_CUSTOMIZE &&
        (s_ui.pressedTag == COFFEE_TAG_SIZE || s_ui.pressedTag == COFFEE_TAG_MILK);
    s_ui.releasedTag = 0;
    if (touching) s_ui.touchX = (uint16_t)(xy >> 16);
    if (touching != s_ui.rawTouching || tag != s_ui.rawTag) {
        s_ui.rawTouching = touching;
        s_ui.rawTag = tag;
        s_ui.rawChanged = s_ui.now;
        if (touching) s_ui.lastActivity = s_ui.now;
        s_ui.dirty = true;
    }
    if (touching && s_ui.pressedTag && tag != s_ui.pressedTag && !sliderDrag) {
        s_ui.pressedTag = 0;
        s_ui.touchCanceled = true;
        s_ui.dirty = true;
    }
    if (touching && tag && !s_ui.pressedTag && !s_ui.touchCanceled &&
        s_ui.now - s_ui.rawChanged >= TOUCH_DEBOUNCE_MS) {
        s_ui.pressedTag = tag;
        s_ui.dirty = true;
    }
    if (!touching && s_ui.now - s_ui.rawChanged >= TOUCH_DEBOUNCE_MS) {
        if (s_ui.pressedTag && !s_ui.touchCanceled)
            s_ui.releasedTag = s_ui.pressedTag;
        if (s_ui.pressedTag || s_ui.touchCanceled) {
            s_ui.pressedTag = 0;
            s_ui.touchCanceled = false;
            s_ui.dirty = true;
        }
    }
}

static void updateSlider(void)
{
    uint16_t x;
    uint16_t minimum, maximum;
    if (!s_ui.rawTouching || (s_ui.pressedTag != COFFEE_TAG_SIZE &&
        s_ui.pressedTag != COFFEE_TAG_MILK)) return;
    x = s_ui.touchX;
    if (x < 492U) x = 492U;
    if (x > 1232U) x = 1232U;
    if (s_ui.pressedTag == COFFEE_TAG_SIZE) {
        minimum = s_drinks[s_ui.drink].minimum; maximum = s_drinks[s_ui.drink].maximum;
        s_ui.volume = (uint16_t)(minimum + (uint32_t)(x - 492U) * (maximum - minimum) / 740U);
    } else {
        s_ui.milkRatio = (uint8_t)((uint32_t)(x - 492U) * 100U / 740U);
    }
    s_ui.dirty = true;
}

static void dispatchRelease(void)
{
    uint8_t tag = s_ui.releasedTag;
    if (!tag) return;
    if (s_ui.page == COFFEE_PAGE_STANDBY) { enterPage(COFFEE_PAGE_HOME); return; }
    if (tag == COFFEE_TAG_LANGUAGE) {
        s_ui.previousPage = s_ui.page; s_ui.languageSelected = false;
        enterPage(COFFEE_PAGE_LANGUAGE); return;
    }
    if (s_ui.page == COFFEE_PAGE_HOME) {
        if (tag >= COFFEE_TAG_DRINK_FIRST && tag <= COFFEE_TAG_DRINK_LAST)
            chooseDrink((CoffeeDrinkId)(tag - COFFEE_TAG_DRINK_FIRST));
        else if (tag == COFFEE_TAG_CLEAN) CoffeeController_RequestClean();
        else if (tag == COFFEE_TAG_HOT_WATER) chooseDrink(COFFEE_DRINK_HOT_WATER);
        else if (tag == COFFEE_TAG_STANDBY) { CoffeeController_RequestStandby(); enterPage(COFFEE_PAGE_STANDBY); }
    } else if (s_ui.page == COFFEE_PAGE_CUSTOMIZE) {
        if (tag == COFFEE_TAG_BACK) enterPage(COFFEE_PAGE_HOME);
        else if (tag >= COFFEE_TAG_STRENGTH_FIRST && tag <= COFFEE_TAG_STRENGTH_LAST)
            s_ui.strength = (uint8_t)(tag - COFFEE_TAG_STRENGTH_FIRST + 1U);
        else if (tag >= COFFEE_TAG_TEMP_FIRST && tag <= COFFEE_TAG_TEMP_LAST)
            s_ui.temperature = (uint8_t)(tag - COFFEE_TAG_TEMP_FIRST);
        else if (tag == COFFEE_TAG_START) startSelectedDrink();
    } else if (s_ui.page == COFFEE_PAGE_QUICK_CONFIRM) {
        if (tag == COFFEE_TAG_BACK) enterPage(COFFEE_PAGE_HOME);
        else if (tag == COFFEE_TAG_START) startSelectedDrink();
    } else if (s_ui.page == COFFEE_PAGE_BREWING && tag == COFFEE_TAG_STOP) {
        CoffeeController_RequestStop();
    } else if (s_ui.page == COFFEE_PAGE_LANGUAGE) {
        if (tag == COFFEE_TAG_ENGLISH || tag == COFFEE_TAG_CHINESE) {
            s_ui.language = tag == COFFEE_TAG_CHINESE ? COFFEE_LANGUAGE_CHINESE : COFFEE_LANGUAGE_ENGLISH;
            s_ui.languageSelected = true;
            (void)CoffeeController_SaveLanguage(s_ui.language);
        } else if (tag == COFFEE_TAG_BACK && s_ui.languageSelected) enterPage(s_ui.previousPage);
    }
    s_ui.dirty = true;
}

static void updateStatus(void)
{
    CoffeeStatus next;
    if (s_ui.now - s_ui.lastStatus < STATIC_STATUS_MS) return;
    s_ui.lastStatus = s_ui.now;
    CoffeeController_ReadStatus(&next);
    if (memcmp(&next, &s_ui.status, sizeof(next)) != 0) {
        s_ui.status = next;
        s_ui.dirty = true;
    }
    if (s_ui.page == COFFEE_PAGE_BREWING) {
        if (s_ui.status.stopConfirmed) enterPage(COFFEE_PAGE_HOME);
        else if (s_ui.status.brewComplete) {
            if (!s_ui.doneAt) s_ui.doneAt = s_ui.now;
            else if (s_ui.now - s_ui.doneAt >= DONE_HOLD_MS) enterPage(COFFEE_PAGE_HOME);
        }
    }
}

static int selfTest(void)
{
    uint8_t i;
    for (i = 0; i < COFFEE_STR_COUNT; ++i) {
        if (!CoffeeStrings_Get(COFFEE_LANGUAGE_ENGLISH, (CoffeeStringId)i)[0] ||
            !CoffeeStrings_Get(COFFEE_LANGUAGE_CHINESE, (CoffeeStringId)i)[0])
            return 1;
    }
    for (i = 0; i < COFFEE_DRINK_COUNT; ++i) {
        if (!CoffeeStrings_Drink(COFFEE_LANGUAGE_ENGLISH, (CoffeeDrinkId)i)[0] ||
            !CoffeeStrings_Drink(COFFEE_LANGUAGE_CHINESE, (CoffeeDrinkId)i)[0] ||
            s_drinks[i].minimum > s_drinks[i].volume || s_drinks[i].volume > s_drinks[i].maximum)
            return 1;
    }
    if (COFFEE_DRINK_RAMG_BASE != COFFEE_FONT_CACHE_RAMG + COFFEE_FONT_CACHE_SIZE ||
        COFFEE_DRINK_RAMG_BASE + COFFEE_DRINK_COUNT * COFFEE_DRINK_SLOT_BYTES > RAM_G_SIZE)
        return 1;
    if (selectTextFont(COFFEE_PAGE_LANGUAGE, 44U, "English") != 31U ||
        selectTextFont(COFFEE_PAGE_LANGUAGE, 28U, "English") != 29U ||
        selectTextFont(COFFEE_PAGE_LANGUAGE, 28U, "\xE4\xB8\xAD") != COFFEE_FONT44_HANDLE ||
        selectTextFont(COFFEE_PAGE_HOME, 24U, "\xE5\x92\x96\xE5\x95\xA1\xE8\xB1\x86") !=
            COFFEE_FONT32_HANDLE ||
        pageCacheFont(COFFEE_PAGE_BREWING) != COFFEE_FONT32_HANDLE ||
        selectTextFont(COFFEE_PAGE_BREWING, 44U, "00:30") != 31U ||
        selectTextFont(COFFEE_PAGE_BREWING, 44U, "\xE6\x84\x8F\xE5\xBC\x8F") !=
            COFFEE_FONT32_HANDLE ||
        remainingSeconds(30U, 0U) != 30U ||
        remainingSeconds(30U, 12U) != 18U ||
        remainingSeconds(30U, 31U) != 0U)
        return 1;
    printf("SELF-TEST PASS: strings, drink ranges, tags, font routing and RAM_G map\n");
    return 0;
}

static void showStartupAssetError(void)
{
    EVE_CoCmd_dlStart(&s_hal);
    EVE_CoDl_clearColorRgb(&s_hal, 0x28, 0x08, 0x08);
    EVE_CoDl_clear(&s_hal, 1, 1, 1);
    EVE_CoDl_colorRgb(&s_hal, 0xFF, 0xFF, 0xFF);
    EVE_CoCmd_text(&s_hal, 640, 350, 27, OPT_CENTER,
        "FLASH ASSET MISMATCH - CHECK SERIAL LOG");
    EVE_CoDl_display(&s_hal);
    if (EVE_Cmd_waitFlush(&s_hal)) {
        EVE_CoCmd_swap(&s_hal);
        (void)EVE_Cmd_waitFlush(&s_hal);
    }
}

static void probeSolidScanout(void)
{
    uint32_t frames, underruns;
    EVE_CoCmd_dlStart(&s_hal);
    EVE_CoDl_clearColorRgb(&s_hal, 0x17, 0x11, 0x0D);
    EVE_CoDl_clear(&s_hal, 1, 1, 1);
    EVE_CoDl_display(&s_hal);
    if (!EVE_Cmd_waitFlush(&s_hal)) {
        fprintf(stderr, "CoffeeMachine: solid scanout probe command failed\n");
        return;
    }
    EVE_CoCmd_swap(&s_hal);
    if (!EVE_Cmd_waitFlush(&s_hal)) {
        fprintf(stderr, "CoffeeMachine: solid scanout probe swap failed\n");
        return;
    }
    EVE_sleep(100);
    frames = EVE_Hal_rd32(&s_hal, REG_FRAMES);
    underruns = EVE_Hal_rd32(&s_hal, REG_UNDERRUN);
    EVE_sleep(1000);
    s_probe[0].frames = EVE_Hal_rd32(&s_hal, REG_FRAMES) - frames;
    s_probe[0].underrun = EVE_Hal_rd32(&s_hal, REG_UNDERRUN) - underruns;
    s_probe[0].valid = true;
    printf("CoffeeMachine probe=solid frames=%lu underrun=%lu hcycle=%lu hsize=%lu vcycle=%lu pclk=%lu adaptive=%lu\n",
        (unsigned long)s_probe[0].frames,
        (unsigned long)s_probe[0].underrun,
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_HCYCLE),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_HSIZE),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_VCYCLE),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_PCLK),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_ADAPTIVE_FRAMERATE));
    printf("CoffeeMachine timing hsync0=%lu hsync1=%lu hoffset=%lu vsync0=%lu vsync1=%lu voffset=%lu vsize=%lu frequency=%lu pclkfreq=%lu pclk2x=%lu ahmax=%lu\n",
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_HSYNC0),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_HSYNC1),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_HOFFSET),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_VSYNC0),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_VSYNC1),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_VOFFSET),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_VSIZE),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_FREQUENCY),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_PCLK_FREQ),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_PCLK_2X),
        (unsigned long)EVE_Hal_rd32(&s_hal, REG_AH_HCYCLE_MAX));
}

static void probeStandbyText(bool includeHint)
{
    uint32_t frames, underruns;
    ScanoutProbe *probe = &s_probe[includeHint ? 2 : 1];
    EVE_CoCmd_dlStart(&s_hal);
    CoffeeAssets_BindFonts(&s_hal);
    EVE_CoCmd_romFont(&s_hal, 29, 29);
    if (!CoffeeAssets_CacheFont(&s_hal, COFFEE_FONT44_HANDLE)) return;
    EVE_CoDl_clearColorRgb(&s_hal, 0x08, 0x06, 0x04);
    EVE_CoDl_clear(&s_hal, 1, 1, 1);
    EVE_CoDl_colorRgb(&s_hal, 0xF5, 0xEE, 0xE8);
    EVE_CoCmd_text(&s_hal, 640, 350, COFFEE_FONT44_HANDLE,
        OPT_CENTER, tr(COFFEE_STR_STANDBY));
    if (includeHint) {
        EVE_CoDl_colorRgb(&s_hal, 0xA8, 0x98, 0x8C);
        EVE_CoCmd_text(&s_hal, 640, 418,
            s_ui.language == COFFEE_LANGUAGE_CHINESE ? COFFEE_FONT44_HANDLE : 29U,
            OPT_CENTER, tr(COFFEE_STR_STANDBY_HINT));
    }
    EVE_CoDl_display(&s_hal);
    if (!EVE_Cmd_waitFlush(&s_hal)) return;
    EVE_CoCmd_swap(&s_hal);
    if (!EVE_Cmd_waitFlush(&s_hal)) return;
    EVE_sleep(100);
    frames = EVE_Hal_rd32(&s_hal, REG_FRAMES);
    underruns = EVE_Hal_rd32(&s_hal, REG_UNDERRUN);
    EVE_sleep(750);
    probe->frames = EVE_Hal_rd32(&s_hal, REG_FRAMES) - frames;
    probe->underrun = EVE_Hal_rd32(&s_hal, REG_UNDERRUN) - underruns;
    probe->valid = true;
    printf("CoffeeMachine probe=%s frames=%lu underrun=%lu\n",
        includeHint ? "standby44+hint" : "standby44",
        (unsigned long)probe->frames,
        (unsigned long)probe->underrun);
}

static void probePageImages(CoffeePage page, bool showImages)
{
    uint32_t frames, underruns;
    CoffeePage previousPage = s_ui.page;
    uint8_t index = page == COFFEE_PAGE_HOME ? 3U : 5U;
    ScanoutProbe *probe = &s_probe[index + (showImages ? 1U : 0U)];
    s_ui.page = page;
    s_probeHideDrinkImages = !showImages;
    if (!drawFrame()) {
        s_probeHideDrinkImages = false;
        s_ui.page = previousPage;
        return;
    }
    EVE_sleep(100);
    frames = EVE_Hal_rd32(&s_hal, REG_FRAMES);
    underruns = EVE_Hal_rd32(&s_hal, REG_UNDERRUN);
    EVE_sleep(750);
    probe->frames = EVE_Hal_rd32(&s_hal, REG_FRAMES) - frames;
    probe->underrun = EVE_Hal_rd32(&s_hal, REG_UNDERRUN) - underruns;
    probe->valid = true;
    printf("CoffeeMachine probe=%s-%s frames=%lu underrun=%lu\n",
        pageName(page),
        showImages ? "images" : "no-images",
        (unsigned long)probe->frames, (unsigned long)probe->underrun);
    s_probeHideDrinkImages = false;
    s_ui.page = previousPage;
}

static void probeBrewingGauge(bool showGauge)
{
    CoffeePage previousPage = s_ui.page;
    CoffeeStatus previousStatus = s_ui.status;
    uint32_t frames, underruns;
    ScanoutProbe *probe = &s_probe[showGauge ? 8 : 7];
    s_ui.page = COFFEE_PAGE_BREWING;
    s_ui.status.state = COFFEE_CONTROLLER_BUSY;
    s_ui.status.heartbeatOk = true;
    s_ui.status.progress = 50;
    s_ui.status.elapsedSeconds = 15;
    s_ui.status.totalSeconds = 30;
    s_ui.status.step = COFFEE_STEP_BREWING;
    s_probeHideBrewGauge = !showGauge;
    if (!drawFrame()) {
        s_probeHideBrewGauge = false;
        s_ui.status = previousStatus;
        s_ui.page = previousPage;
        return;
    }
    EVE_sleep(100);
    frames = EVE_Hal_rd32(&s_hal, REG_FRAMES);
    underruns = EVE_Hal_rd32(&s_hal, REG_UNDERRUN);
    EVE_sleep(500);
    probe->frames = EVE_Hal_rd32(&s_hal, REG_FRAMES) - frames;
    probe->underrun = EVE_Hal_rd32(&s_hal, REG_UNDERRUN) - underruns;
    probe->valid = true;
    printf("CoffeeMachine probe=brewing-%s frames=%lu underrun=%lu\n",
        showGauge ? "ring" : "no-ring",
        (unsigned long)probe->frames, (unsigned long)probe->underrun);
    s_probeHideBrewGauge = false;
    s_ui.status = previousStatus;
    s_ui.page = previousPage;
}

#if defined(_WIN32)
int main(int argc, char *argv[])
#else
int main(void)
#endif
{
    static const CoffeePage smokePages[] = {
        COFFEE_PAGE_HOME, COFFEE_PAGE_CUSTOMIZE, COFFEE_PAGE_QUICK_CONFIRM,
        COFFEE_PAGE_LANGUAGE, COFFEE_PAGE_BREWING, COFFEE_PAGE_STANDBY
    };
    bool running = true;
    bool smoke = false;
    bool smokeChinese = false;
    bool failed = false;
    bool smokeDrawn = false;
    uint8_t smokePhase = 0;
    uint32_t smokePageStarted = 0;
#if defined(_WIN32)
    if (argc > 1 && strcmp(argv[1], "--self-test") == 0) return selfTest();
    smoke = argc > 1 && (strcmp(argv[1], "--smoke") == 0 ||
        strcmp(argv[1], "--smoke-zh") == 0);
    smokeChinese = argc > 1 && strcmp(argv[1], "--smoke-zh") == 0;
#endif
    memset(&s_ui, 0, sizeof(s_ui));
    s_ui.page = COFFEE_PAGE_HOME;
    s_ui.language = COFFEE_LANGUAGE_ENGLISH;
    (void)CoffeeController_LoadLanguage(&s_ui.language);
    if (smokeChinese) s_ui.language = COFFEE_LANGUAGE_CHINESE;
    chooseDrink(COFFEE_DRINK_ESPRESSO);
    s_ui.page = COFFEE_PAGE_HOME;
    s_ui.dirty = true;
    if (!CoffeeMachine_PlatformInit(&s_hal)) return 1;
    printf("CoffeeMachine: EVE initialization complete\n");
    if (s_hal.Width != COFFEE_SCREEN_WIDTH || s_hal.Height != COFFEE_SCREEN_HEIGHT) {
        fprintf(stderr, "CoffeeMachine requires 1280x800, got %ux%u\n", s_hal.Width, s_hal.Height);
        CoffeeMachine_PlatformRelease(&s_hal); return 1;
    }
    probeSolidScanout();
    if (!CoffeeAssets_Load(&s_hal)) {
        showStartupAssetError();
        /* Keep RP2040 USB CDC alive for diagnosis instead of returning from
         * main when the EVE flash image is absent or revision-mismatched. */
        while (CoffeeMachine_PlatformPump()) EVE_sleep(50);
        CoffeeMachine_PlatformRelease(&s_hal);
        return 1;
    }
    /* Home and Customize probes render before the main polling loop. Use a
     * real controller status so their first frames do not show a false alert. */
    CoffeeController_ReadStatus(&s_ui.status);
    probeStandbyText(false);
    probeStandbyText(true);
    probePageImages(COFFEE_PAGE_HOME, false);
    probePageImages(COFFEE_PAGE_HOME, true);
    probePageImages(COFFEE_PAGE_CUSTOMIZE, false);
    probePageImages(COFFEE_PAGE_CUSTOMIZE, true);
    probeBrewingGauge(false);
    probeBrewingGauge(true);
    s_ui.now = EVE_millis(); s_ui.lastActivity = s_ui.now;
    s_ui.lastUnderrunCheck = s_ui.now;
    s_ui.lastUnderrun = EVE_Hal_rd32(&s_hal, REG_UNDERRUN);
    s_ui.lastFrames = EVE_Hal_rd32(&s_hal, REG_FRAMES);
    s_ui.lastProbeReplay = s_ui.now - PROBE_REPLAY_MS;
    while (running && CoffeeMachine_PlatformPump()) {
        s_ui.now = EVE_millis();
        updateStatus(); sampleTouch(); updateSlider(); dispatchRelease();
        if (s_ui.page == COFFEE_PAGE_HOME && s_ui.now - s_ui.lastActivity >= HOME_IDLE_MS)
            enterPage(COFFEE_PAGE_STANDBY);
        if (smoke) {
            if (smokeDrawn && (uint32_t)(s_ui.now - smokePageStarted) >= 700U) {
                ++smokePhase;
                smokeDrawn = false;
                if (smokePhase == sizeof(smokePages) / sizeof(smokePages[0]))
                    running = false;
                else
                    enterPage(smokePages[smokePhase]);
            }
            if (running && s_ui.page != smokePages[smokePhase])
                enterPage(smokePages[smokePhase]);
        }
        if (running && s_ui.dirty) {
            if (!drawFrame()) { failed = true; running = false; break; }
            s_ui.dirty = false; s_ui.lastDraw = s_ui.now;
            if (smoke && !smokeDrawn) {
                smokeDrawn = true;
                smokePageStarted = s_ui.now;
                printf("CoffeeMachine smoke page=%s rendered\n", pageName(s_ui.page));
            }
        }
        monitorUnderrun();
        EVE_sleep(5);
    }
    CoffeeMachine_PlatformRelease(&s_hal);
    if (failed || (smoke && smokePhase != sizeof(smokePages) / sizeof(smokePages[0])))
        return 1;
    return 0;
}
