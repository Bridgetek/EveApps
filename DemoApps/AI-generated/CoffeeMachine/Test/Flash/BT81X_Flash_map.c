#include <stdint.h>

/* Flash asset info structure */
typedef struct {
    uint32_t offset;
    uint32_t size;
} eve_flash_asset_size_info;

/* Flash asset index enum */
typedef enum {
    default_fl_blob = 0,
    EDF_Block = 1,
    NotoSansCJK_Regular_24_ASTC_xfont = 2,
    NotoSansCJK_Regular_24_ASTC_glyph = 3,
    NotoSansCJK_Regular_28_ASTC_xfont = 4,
    NotoSansCJK_Regular_28_ASTC_glyph = 5,
    NotoSansCJK_Regular_32_ASTC_xfont = 6,
    NotoSansCJK_Regular_32_ASTC_glyph = 7,
    NotoSansCJK_Regular_44_ASTC_xfont = 8,
    NotoSansCJK_Regular_44_ASTC_glyph = 9,
    NotoSansCJK_Regular_84_ASTC_xfont = 10,
    NotoSansCJK_Regular_84_ASTC_glyph = 11,
    espresso_raw = 12,
    lungo_raw = 13,
    americano_raw = 14,
    cappuccino_raw = 15,
    latte_raw = 16,
    flat_white_raw = 17,
    hot_water_raw = 18,
    milk_foam_raw = 19,
    flash_asset_index_last
} flash_asset_index;

/* Flash asset list array */
eve_flash_asset_size_info flash_asset_list[flash_asset_index_last] =
{
    [default_fl_blob] = {
        .offset = 0,
        .size = 4096
    },
    [EDF_Block] = {
        .offset = 4096,
        .size = 364
    },
    [NotoSansCJK_Regular_24_ASTC_xfont] = {
        .offset = 4480,
        .size = 3760
    },
    [NotoSansCJK_Regular_24_ASTC_glyph] = {
        .offset = 8256,
        .size = 851200
    },
    [NotoSansCJK_Regular_28_ASTC_xfont] = {
        .offset = 859456,
        .size = 3760
    },
    [NotoSansCJK_Regular_28_ASTC_glyph] = {
        .offset = 863232,
        .size = 851200
    },
    [NotoSansCJK_Regular_32_ASTC_xfont] = {
        .offset = 1714432,
        .size = 3760
    },
    [NotoSansCJK_Regular_32_ASTC_glyph] = {
        .offset = 1718208,
        .size = 1094400
    },
    [NotoSansCJK_Regular_44_ASTC_xfont] = {
        .offset = 2812608,
        .size = 3760
    },
    [NotoSansCJK_Regular_44_ASTC_glyph] = {
        .offset = 2816384,
        .size = 1094400
    },
    [NotoSansCJK_Regular_84_ASTC_xfont] = {
        .offset = 3910784,
        .size = 3760
    },
    [NotoSansCJK_Regular_84_ASTC_glyph] = {
        .offset = 3914560,
        .size = 3404800
    },
    [espresso_raw] = {
        .offset = 7319360,
        .size = 68400
    },
    [lungo_raw] = {
        .offset = 7387776,
        .size = 68400
    },
    [americano_raw] = {
        .offset = 7456192,
        .size = 68400
    },
    [cappuccino_raw] = {
        .offset = 7524608,
        .size = 68400
    },
    [latte_raw] = {
        .offset = 7593024,
        .size = 68400
    },
    [flat_white_raw] = {
        .offset = 7661440,
        .size = 68400
    },
    [hot_water_raw] = {
        .offset = 7729856,
        .size = 68400
    },
    [milk_foam_raw] = {
        .offset = 7798272,
        .size = 68400
    }
};

/* Total flash asset size */
// Can be read using extern declaration
const uint32_t total_flash_asset = flash_asset_index_last;
