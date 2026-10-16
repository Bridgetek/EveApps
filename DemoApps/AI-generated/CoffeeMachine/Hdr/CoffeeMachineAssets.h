#ifndef COFFEE_MACHINE_ASSETS_H_
#define COFFEE_MACHINE_ASSETS_H_

#include <stdbool.h>
#include <stdint.h>
#include "CoffeeMachine.h"
#include "Platform.h"

#define COFFEE_FONT24_HANDLE 8U
#define COFFEE_FONT28_HANDLE 9U
#define COFFEE_FONT32_HANDLE 10U
#define COFFEE_FONT44_HANDLE 11U
#define COFFEE_FONT84_HANDLE 12U

/* RAM_G: XFONT [0,20480), font cache [20480,479232), eight aligned
 * ASTC images [479232,1026560), free [1026560,1048576). */
#define COFFEE_FONT24_RAMG 0UL
#define COFFEE_FONT28_RAMG 4096UL
#define COFFEE_FONT32_RAMG 8192UL
#define COFFEE_FONT44_RAMG 12288UL
#define COFFEE_FONT84_RAMG 16384UL
#define COFFEE_FONT_CACHE_RAMG 20480UL
#define COFFEE_FONT_CACHE_SIZE (448UL * 1024UL)
#define COFFEE_DRINK_RAMG_BASE 479232UL
#define COFFEE_DRINK_SLOT_BYTES 68416UL
#define COFFEE_DRINK_RAW_BYTES 68400UL
#define COFFEE_DRINK_WIDTH 600U
#define COFFEE_DRINK_HEIGHT 456U

/* Exact offsets and descriptor sizes from Test/Flash/BT81X_Flash.map. */
#define COFFEE_FONT24_FLASH 4480UL
#define COFFEE_FONT24_GLYPH_FLASH 8256UL
#define COFFEE_FONT28_FLASH 859456UL
#define COFFEE_FONT28_GLYPH_FLASH 863232UL
#define COFFEE_FONT32_FLASH 1714432UL
#define COFFEE_FONT32_GLYPH_FLASH 1718208UL
#define COFFEE_FONT44_FLASH 2812608UL
#define COFFEE_FONT44_GLYPH_FLASH 2816384UL
#define COFFEE_FONT84_FLASH 3910784UL
#define COFFEE_FONT84_GLYPH_FLASH 3914560UL
#define COFFEE_FONT24_XFONT_SIZE 3760UL
#define COFFEE_FONT28_XFONT_SIZE 3760UL
#define COFFEE_FONT32_XFONT_SIZE 3760UL
#define COFFEE_FONT44_XFONT_SIZE 3760UL
#define COFFEE_FONT84_XFONT_SIZE 3760UL

extern const uint32_t g_CoffeeDrinkFlash[COFFEE_DRINK_COUNT];

bool CoffeeAssets_Load(EVE_HalContext *host);
void CoffeeAssets_BindFonts(EVE_HalContext *host);
bool CoffeeAssets_CacheFont(EVE_HalContext *host, uint8_t handle);
uint8_t CoffeeAssets_Font(uint16_t pixels);
void CoffeeAssets_DrawDrink(EVE_HalContext *host, CoffeeDrinkId drink,
    int16_t x, int16_t y, uint16_t width, uint16_t height, bool bilinear);

#endif
