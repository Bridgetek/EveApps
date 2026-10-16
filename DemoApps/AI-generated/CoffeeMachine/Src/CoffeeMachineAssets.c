#include <stdio.h>
#include "Common.h"
#include "EVE_CoCmd.h"
#include "CoffeeMachineAssets.h"

#if EVE_SUPPORT_CHIPID != EVE_BT817
#error CoffeeMachine requires the exact EVE_GRAPHICS_BT817 target.
#endif

/* Exact offsets from Test/Flash/BT81X_Flash.map, in CoffeeDrinkId order. */
const uint32_t g_CoffeeDrinkFlash[COFFEE_DRINK_COUNT] = {
    7319360UL, 7387776UL, 7456192UL, 7524608UL,
    7593024UL, 7661440UL, 7729856UL, 7798272UL
};

/* CRC-32 of the EAB ASTC_8x8 raw files in CoffeeDrinkId order. */
static const uint32_t s_drinkCrc[COFFEE_DRINK_COUNT] = {
    0xA7D9E2A8UL, 0x526542F4UL, 0x3971F56DUL, 0xD55FFE92UL,
    0xF6F98613UL, 0xF9229E57UL, 0x1F35BD9AUL, 0xA7D373D6UL
};

typedef struct FontAsset {
    uint32_t flash;
    uint32_t glyphFlash;
    uint32_t ramg;
    uint32_t size;
    uint8_t handle;
} FontAsset;

static const FontAsset s_fonts[] = {
    { COFFEE_FONT24_FLASH, COFFEE_FONT24_GLYPH_FLASH, COFFEE_FONT24_RAMG, COFFEE_FONT24_XFONT_SIZE, COFFEE_FONT24_HANDLE },
    { COFFEE_FONT28_FLASH, COFFEE_FONT28_GLYPH_FLASH, COFFEE_FONT28_RAMG, COFFEE_FONT28_XFONT_SIZE, COFFEE_FONT28_HANDLE },
    { COFFEE_FONT32_FLASH, COFFEE_FONT32_GLYPH_FLASH, COFFEE_FONT32_RAMG, COFFEE_FONT32_XFONT_SIZE, COFFEE_FONT32_HANDLE },
    { COFFEE_FONT44_FLASH, COFFEE_FONT44_GLYPH_FLASH, COFFEE_FONT44_RAMG, COFFEE_FONT44_XFONT_SIZE, COFFEE_FONT44_HANDLE },
    { COFFEE_FONT84_FLASH, COFFEE_FONT84_GLYPH_FLASH, COFFEE_FONT84_RAMG, COFFEE_FONT84_XFONT_SIZE, COFFEE_FONT84_HANDLE }
};

static uint8_t s_cachedFont = 255U;
static bool s_assetsLoaded;

static bool copyDrink(EVE_HalContext *host, uint8_t i)
{
    uint32_t ramg = COFFEE_DRINK_RAMG_BASE + (uint32_t)i * COFFEE_DRINK_SLOT_BYTES;
    uint32_t crc;
    if ((g_CoffeeDrinkFlash[i] & 63UL) || (ramg & 63UL)) return false;
    EVE_CoCmd_flashRead(host, ramg, g_CoffeeDrinkFlash[i], COFFEE_DRINK_RAW_BYTES);
    return EVE_Cmd_waitFlush(host) &&
        EVE_CoCmd_memCrc(host, ramg, COFFEE_DRINK_RAW_BYTES, &crc) &&
        crc == s_drinkCrc[i];
}

bool CoffeeAssets_Load(EVE_HalContext *host)
{
    uint8_t i;
    s_assetsLoaded = false;
    if (!COFFEE_FONT24_XFONT_SIZE || !g_CoffeeDrinkFlash[COFFEE_DRINK_ESPRESSO]) {
        fprintf(stderr, "CoffeeMachine: generated asset map is missing\n");
        return false;
    }
    if (COFFEE_FONT24_XFONT_SIZE > 4096UL || COFFEE_FONT28_XFONT_SIZE > 4096UL ||
        COFFEE_FONT32_XFONT_SIZE > 4096UL || COFFEE_FONT44_XFONT_SIZE > 4096UL ||
        COFFEE_FONT84_XFONT_SIZE > 4096UL ||
        COFFEE_FONT84_RAMG + 4096UL > COFFEE_FONT_CACHE_RAMG ||
        COFFEE_FONT_CACHE_RAMG + COFFEE_FONT_CACHE_SIZE > COFFEE_DRINK_RAMG_BASE ||
        COFFEE_DRINK_RAMG_BASE + COFFEE_DRINK_COUNT * COFFEE_DRINK_SLOT_BYTES > RAM_G_SIZE ||
        COFFEE_DRINK_SLOT_BYTES < COFFEE_DRINK_RAW_BYTES ||
        (COFFEE_FONT_CACHE_RAMG & 63UL) || (COFFEE_DRINK_RAMG_BASE & 63UL)) {
        fprintf(stderr, "CoffeeMachine: invalid BT817 RAM_G resource map\n");
        return false;
    }
#if defined(EVE_SUPPORT_FLASH)
    if (!FlashHelper_SwitchFullMode(host)) {
        fprintf(stderr, "CoffeeMachine: external flash did not enter FULL mode\n");
        return false;
    }
    for (i = 0; i < (uint8_t)(sizeof(s_fonts) / sizeof(s_fonts[0])); ++i) {
        EVE_CoCmd_flashRead(host, s_fonts[i].ramg, s_fonts[i].flash,
            (s_fonts[i].size + 3UL) & ~3UL);
    }
    if (!EVE_Cmd_waitFlush(host)) return false;
    for (i = 0; i < (uint8_t)(sizeof(s_fonts) / sizeof(s_fonts[0])); ++i) {
        if (EVE_Hal_rd32(host, s_fonts[i].ramg) != 0x0100AAFFUL ||
            EVE_Hal_rd32(host, s_fonts[i].ramg + 4UL) != s_fonts[i].size ||
            EVE_Hal_rd32(host, s_fonts[i].ramg + 32UL) != ATFLASH(s_fonts[i].glyphFlash)) {
            fprintf(stderr, "CoffeeMachine: flash image/XFONT map mismatch at handle %u\n",
                (unsigned)s_fonts[i].handle);
            return false;
        }
    }
    for (i = 0; i < (uint8_t)(sizeof(s_fonts) / sizeof(s_fonts[0])); ++i)
        EVE_CoCmd_setFont2(host, s_fonts[i].handle, s_fonts[i].ramg, 0);
    if (!EVE_Cmd_waitFlush(host)) return false;
    for (i = 0; i < COFFEE_DRINK_COUNT; ++i) {
        if (!copyDrink(host, i)) {
            fprintf(stderr, "CoffeeMachine: ASTC8x8 drink %u flash image/CRC mismatch\n",
                (unsigned)i);
            return false;
        }
    }
    printf("CoffeeMachine: loaded ASTC8x8 flash assets into RAM_G\n");
    s_cachedFont = 255U;
    s_assetsLoaded = true;
    return true;
#else
    (void)host;
    return false;
#endif
}

void CoffeeAssets_BindFonts(EVE_HalContext *host)
{
    uint8_t i;
    /* Custom bitmap-handle state belongs to the current display list. */
    for (i = 0; i < (uint8_t)(sizeof(s_fonts) / sizeof(s_fonts[0])); ++i)
        EVE_CoCmd_setFont2(host, s_fonts[i].handle, s_fonts[i].ramg, 0);
}

bool CoffeeAssets_CacheFont(EVE_HalContext *host, uint8_t handle)
{
    uint32_t total;
    int32_t used;
    if (handle < COFFEE_FONT24_HANDLE || handle > COFFEE_FONT84_HANDLE)
        return false;
    if (handle == s_cachedFont) return true;
    if (!s_assetsLoaded) return false;
    EVE_CoCmd_fontCache(host, handle, COFFEE_FONT_CACHE_RAMG, COFFEE_FONT_CACHE_SIZE);
    if (!EVE_Cmd_waitFlush(host) ||
        !EVE_CoCmd_fontCacheQuery(host, &total, &used) || !total) {
        fprintf(stderr, "CoffeeMachine: BT817 font cache setup failed for handle %u\n",
            (unsigned)handle);
        return false;
    }
    printf("CoffeeMachine font cache handle=%u slots=%lu used=%ld\n",
        (unsigned)handle, (unsigned long)total, (long)used);
    s_cachedFont = handle;
    return true;
}

uint8_t CoffeeAssets_Font(uint16_t pixels)
{
    if (pixels <= 24U) return COFFEE_FONT24_HANDLE;
    if (pixels <= 28U) return COFFEE_FONT28_HANDLE;
    if (pixels <= 32U) return COFFEE_FONT32_HANDLE;
    if (pixels <= 44U) return COFFEE_FONT44_HANDLE;
    return COFFEE_FONT84_HANDLE;
}

void CoffeeAssets_DrawDrink(EVE_HalContext *host, CoffeeDrinkId drink,
    int16_t x, int16_t y, uint16_t width, uint16_t height, bool bilinear)
{
    int32_t scaleX;
    int32_t scaleY;
    if ((unsigned)drink >= COFFEE_DRINK_COUNT || !s_assetsLoaded) return;
    scaleX = ((int32_t)width << 16) / COFFEE_DRINK_WIDTH;
    scaleY = ((int32_t)height << 16) / COFFEE_DRINK_HEIGHT;
    EVE_CoDl_saveContext(host);
    /* COLOR_RGB modulates bitmap pixels. The preceding dark card must not
     * tint the drink image. */
    EVE_CoDl_colorRgb(host, 255, 255, 255);
    EVE_CoDl_bitmapHandle(host, 0);
    EVE_CoCmd_setBitmap(host,
        COFFEE_DRINK_RAMG_BASE + (uint32_t)drink * COFFEE_DRINK_SLOT_BYTES,
        COMPRESSED_RGBA_ASTC_8x8_KHR, COFFEE_DRINK_WIDTH, COFFEE_DRINK_HEIGHT);
    EVE_CoDl_bitmapSize(host, bilinear ? BILINEAR : NEAREST,
        BORDER, BORDER, width, height);
    EVE_CoCmd_loadIdentity(host);
    EVE_CoCmd_scale(host, scaleX, scaleY);
    EVE_CoCmd_setMatrix(host);
    EVE_CoDl_begin(host, BITMAPS);
    EVE_CoDl_vertex2f(host, x, y);
    EVE_CoDl_end(host);
    EVE_CoDl_restoreContext(host);
}
