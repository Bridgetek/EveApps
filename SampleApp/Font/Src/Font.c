/**
 * @file Font.c
 * @brief Sample usage of font
 *
 * @author Bridgetek
 *
 * @date 2019
 * 
 * MIT License
 *
 * Copyright (c) [2019] [Bridgetek Pte Ltd (BRTChip)]
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Common.h"
#include "Platform.h"
#include "EVE_CoCmd.h"
#include "Font.h"

#define SAMAPP_INFO_TEXT(str)  Draw_TextColor(s_pHalContext, str, (uint8_t[]) { 0x77, 0x77, 0x77 }, (uint8_t[]) { 255, 255, 255 })
#define SAMAPP_INFO_START      Display_StartColor(s_pHalContext, (uint8_t[]) { 0x77, 0x77, 0x77 }, (uint8_t[]) { 255, 255, 255 })
#define SAMAPP_INFO_END        Display_End(s_pHalContext);
#define SAMAPP_DELAY_NEXT      EVE_sleep(2000);
#define UNICODE_HANDLE1 (1)
#define UNICODE_HANDLE (30)

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void SAMAPP_Font();

int main(int argc, char* argv[])
{
    s_pHalContext = &s_halContext;
    Gpu_Init(s_pHalContext);

    // read and store calibration setting
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
    Esd_Calibrate(s_pHalContext);
    Calibration_Save(s_pHalContext);
#endif

    // Write flash binary
    Flash_Init(s_pHalContext, TEST_DIR "/Flash/BT81X_Flash.bin", "BT81X_Flash.bin");
    EVE_Util_clearScreen(s_pHalContext);

    char *info[] =
    {  "EVE Sample Application",
        "This sample demonstrate the using of font", 
        "",
        ""
    }; 

    while (TRUE) {
        WelcomeScreen(s_pHalContext, info);

        SAMAPP_Font();

        EVE_Util_clearScreen(s_pHalContext);

        Gpu_Release(s_pHalContext);

        /* Init HW Hal for next loop*/
        Gpu_Init(s_pHalContext);
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
        Calibration_Restore(s_pHalContext);
#endif
    }

    return 0;
}

/**
* @brief Load xfont to RAM_G
*
* @param isReindex
*/
void helperLoadXfont(uint8_t isReindex)
{
#if EVE_SUPPORT_GEN >= EVE3
    static uint32_t unicodeHandle = 0;
    static uint32_t XFONT_ADDR = RAM_G;
    static uint32_t GLYPH_ADDR = 20480; // glyph offset from 20kb
    uint8_t* str_noindex = u8"自來水, 磨蝕性";// it's mean: water wears away the stone
    uint8_t* str_reindex = u8"\x0043\x003f\x0041\x0002\x0001\x0042\x0044\x0040\x0001";
    uint8_t* str = str_noindex;

    const uint32_t xfont_path_noindex[] = {5064704 , 16064 }; // simkai1.xfo
    const uint32_t glyph_path_noindex[] = {4463744 , 600960}; // simkai1.gly
    const uint32_t xfont_path_reindex[] = {5143488 , 128   }; // simkai2.xfo
    const uint32_t glyph_path_reindex[] = {5080768 , 62720 }; // simkai2.gly
    const uint32_t* xfont = xfont_path_noindex;
    const uint32_t* glyph = glyph_path_noindex;
    if (isReindex)
    {
        str = str_reindex;
        xfont = xfont_path_reindex;
        glyph = glyph_path_reindex;
    }

    //Load xfont file into graphics RAM
    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("Cannot switch flash full mode");
        return;
    }
    EVE_CoCmd_flashRead(s_pHalContext, XFONT_ADDR, xfont[0], xfont[1]);
    EVE_Cmd_waitFlush(s_pHalContext);
    //Load glyph file into graphics RAM
    EVE_CoCmd_flashRead(s_pHalContext, GLYPH_ADDR, glyph[0], glyph[1]);
    EVE_Cmd_waitFlush(s_pHalContext);
    
    EVE_CoCmd_gradientA(s_pHalContext, 0, 0, 0xff000000, 0, (int16_t) s_pHalContext->Height,
        0xff999999);

    EVE_CoCmd_setFont2(s_pHalContext, unicodeHandle, XFONT_ADDR, 0);
    if (isReindex)
    {
        EVE_CoCmd_text(s_pHalContext, 10, 10, 29, 0,
            "Display unicode string use re-index font data");
    }
    else
    {
        EVE_CoCmd_text(s_pHalContext, 10, 10, 29, 0, 
            "Display unicode string use font's index");
    }
    EVE_CoCmd_text(s_pHalContext, 10, 70, (int16_t) unicodeHandle, 0, str);
#endif // #if defined (BT81X_ENABLE) && (defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM))
}

/**
* @brief Inbuilt font example for proportionate and non proportionate text
*/
void SAMAPP_Font_romFonts()
{
    int32_t j;
    int32_t hoffset;
    int32_t voffset;
    int32_t stringlen1;
    int32_t stringlen2;
    int32_t stringlen3;
    uint32_t FontTableAddress;
    const uchar8_t Display_string1[] = "!\"#$%&'()*+,-./0123456789:;<=>?";
    const uchar8_t Display_string2[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";
    const uchar8_t Display_string3[] = "`abcdefghijklmnopqrstuvwxyz{|}  ";
    Gpu_Fonts_t Display_fontstruct;
	uint32_t fontSize = 31;

    Draw_Text(s_pHalContext, "Example for: ROM fonts 16 to 34");

    /* Read the font address from 0xFFFFC location */
    FontTableAddress = EVE_Hal_rd32(s_pHalContext, ROMFONT_TABLEADDRESS);
    printf("ROM Font address read from 0x%lx is %x \n", ROMFONT_TABLEADDRESS, FontTableAddress);

    stringlen1 = sizeof(Display_string1) - 1;
    stringlen2 = sizeof(Display_string2) - 1;
    stringlen3 = sizeof(Display_string3) - 1;

    // FT81X and BT81X has rom font 16-34, 32-34 accessible via Cmd_romFont
    // FT80X has rom font 16-31
#if defined(FT81X_ENABLE)
	fontSize = 34;
#endif
	for (uint32_t romFont = 16; romFont <= fontSize; romFont++)
    {
        /* Read the font table from hardware */
        EVE_Hal_rdMem(s_pHalContext, (uint8_t*) &Display_fontstruct,
            (FontTableAddress + (romFont-16) * GPU_FONT_TABLE_SIZE), GPU_FONT_TABLE_SIZE);
        printf("Inbuilt font structure idx %d fmt %d ht %d wd %d strd %d Fontaddr %x \n", romFont,
            Display_fontstruct.FontBitmapFormat, Display_fontstruct.FontHeightInPixels,
            Display_fontstruct.FontLineStride, Display_fontstruct.FontWidthInPixels,
            Display_fontstruct.PointerToFontGraphicsData);
        uint32_t font = romFont % 32;
		Display_StartColor(s_pHalContext, (uint8_t[]) { 0, 0, 0 }, (uint8_t[]) { 255, 255, 255 });
        EVE_CoCmd_romFont(s_pHalContext, font, romFont);
		EVE_Cmd_waitFlush(s_pHalContext);

        /* Display string at the center of display */
        EVE_CoDl_begin(s_pHalContext, BITMAPS);
        hoffset = 20;
        voffset = 0;

        //FT81X devices support larger rom fonts fonts in font handle 32, 33, 34
        /* Display hello world by offsetting wrt char size */
        EVE_CoDl_bitmapHandle(s_pHalContext, font);
        for (j = 0; j < stringlen1; j++)
        {
            EVE_CoDl_cell(s_pHalContext, Display_string1[j]);
            EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset));
            hoffset += Display_fontstruct.FontWidth[Display_string1[j]];
        }
        hoffset = 20;
        voffset += Display_fontstruct.FontHeightInPixels + 3;
        for (j = 0; j < stringlen2; j++)
        {
            EVE_CoDl_cell(s_pHalContext, Display_string2[j]);
            EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset));
            hoffset += Display_fontstruct.FontWidth[Display_string2[j]];
        }
        hoffset = 20;
        voffset += Display_fontstruct.FontHeightInPixels + 3;
        for (j = 0; j < stringlen3; j++)
        {
            EVE_CoDl_cell(s_pHalContext, Display_string3[j]);
            EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset));
            hoffset += Display_fontstruct.FontWidth[Display_string3[j]];
        }

        EVE_CoDl_end(s_pHalContext);

        Display_End(s_pHalContext);
        EVE_sleep(1000);
    }
}

/**
* @brief API to demonstrate CMD_FONTCACHE
*/
void SAMAPP_Font_font_Cache() {
#if EVE_SUPPORT_GEN == EVE4
    const uint32_t xfontOnRamG = RAM_G;
    const uint32_t glyphOnRamG = 4096;
    const uint32_t startOfCache = 800 * 1024;
    const uint32_t cacheSize = 160 * 1024;
    const int loopDraw = 10;
    const int loopCmdText = 10;
    const uint32_t firstchar = 0;
    uint32_t startms;
    uint32_t endms;
    uint32_t x = 10;
    uint32_t y = 10;
    uint32_t duration[2];
    uint32_t fontHandler = UNICODE_HANDLE1;

    /// Xfont address and size from .map file
    const int _add = 0;
    const int _size = 1;

    int xfont_chinese_ASTC[] = { 7680  , 192 };
    int glyph_chinese_ASTC[] = { 4096  , 3584 };

    char* intro[2] = { "ASTC font format without fontcache", "ASTC font format with fontcache" };
    const char* str = u8"\x0004\x0003\x0009\x0005\x000b\x0006\x0008\x000c\x000d\x0001\x0002\x0007";

    Draw_Text(s_pHalContext, "Example for: Font cache");

    // read .xfont to ramg
    APP_INF("Copying xfont from %d by %d bytes to %d", xfont_chinese_ASTC[_add], xfont_chinese_ASTC[_size], xfontOnRamG);
    EVE_CoCmd_flashRead(s_pHalContext, xfontOnRamG, xfont_chinese_ASTC[_add], xfont_chinese_ASTC[_size]);

    EVE_Cmd_waitFlush(s_pHalContext);

    for (int t = 0; t < 2; t++) {
        if (t == 0) { /// without font cache
            fontHandler = UNICODE_HANDLE1;
            SAMAPP_INFO_START;
            EVE_CoCmd_setFont2(s_pHalContext, fontHandler, xfontOnRamG, firstchar);
            SAMAPP_INFO_END;
            EVE_Hal_flush(s_pHalContext);
        }
        else { /// with font cache
            /// Now set fontcache
            SAMAPP_INFO_START;
            fontHandler = t + 1;
            EVE_CoCmd_setFont2(s_pHalContext, fontHandler, xfontOnRamG, firstchar);
            EVE_CoCmd_fontCache(s_pHalContext, fontHandler, startOfCache, cacheSize);
            SAMAPP_INFO_END;
            EVE_Hal_flush(s_pHalContext);
        }

        /// duration without font cache
        startms = EVE_millis();
        for (int i = 0; i < loopDraw; i++) {
            x = y = 20;
            SAMAPP_INFO_START;
            EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));

            EVE_CoCmd_text(s_pHalContext, 10, 10, 30, 0, intro[t]);
            /// Draw unicode string
            for (int k = 0; k < loopCmdText; k++) {
                y += 50;
                EVE_CoCmd_text(s_pHalContext, (uint16_t)x, (uint16_t)y, (uint16_t)fontHandler, 0, str);
            }
            SAMAPP_INFO_END;
        }
        endms = EVE_millis();
        EVE_sleep(2000);
        duration[t] = endms - startms;
    }
    EVE_Cmd_waitFlush(s_pHalContext);
    
    char strDuration[100];
    sprintf_s(strDuration, 100,
        "Duration No FontCache: %u ms\n"
        "Duration With FontCache: %u ms\n"
        "Different: %d ms (%d%%)",
        duration[0], duration[1],
        (int)(duration[0] - duration[1]),
        (int)(abs((int)duration[1] - (int)duration[0]) * 100) / duration[1]);
    SAMAPP_INFO_TEXT(strDuration);

    SAMAPP_DELAY_NEXT;
#endif // EVE_SUPPORT_GEN == EVE4
}

/**
* @brief API to demonstrate CMD_FONTCACHEQUERY
*/
void SAMAPP_Font_fontCacheQuery() {
#if EVE_SUPPORT_GEN == EVE4

    Draw_Text(s_pHalContext, "Example for: CMD_FONTCACHEQUERY");

    const uint32_t xfontOnRamG = RAM_G;
    const uint32_t glyphOnRamG = 4096;
    const uint32_t startOfCache = 800 * 1024;
    const uint32_t cacheSize = 160 * 1024;
    uint32_t x = 10;
    uint32_t y = 10;

    /// Xfont address and size from .map file
    const int _add = 0;
    const int _size = 1;

    int xfont_chinese_ASTC[] = { 7680  , 192 };
    int glyph_chinese_ASTC[] = { 4096  , 3584 };

    const char* str = u8"\x0004\x0003\x0009\x0005\x000b\x0006\x0008\x000c\x000d\x0001\x0002\x0007";
    int loopText = strlen(str);

    // read .xfont to ramg
    APP_DBG("Copying xfont from %d by %d bytes to %d", xfont_chinese_ASTC[_add], xfont_chinese_ASTC[_size], xfontOnRamG);
    EVE_CoCmd_flashRead(s_pHalContext, xfontOnRamG, xfont_chinese_ASTC[_add], xfont_chinese_ASTC[_size]);
    EVE_Cmd_waitFlush(s_pHalContext);

    /// Prepare font handle and cahe font
    SAMAPP_INFO_START;
    EVE_CoCmd_setFont2(s_pHalContext, UNICODE_HANDLE1, xfontOnRamG, 0);
    EVE_CoCmd_fontCache(s_pHalContext, UNICODE_HANDLE1, startOfCache, cacheSize);
    SAMAPP_INFO_END;

    /// Query
    SAMAPP_INFO_START;
    uint32_t totalBefore;
    uint32_t totalAfter;
    int32_t usedBefore;
    int32_t usedAfter;
    EVE_CoCmd_fontCacheQuery(s_pHalContext, &totalBefore, &usedBefore); /* NOTE: Returns false in case of coprocessor fault, output values will be uninitialized */
    SAMAPP_INFO_END;

    /// Draw unicode string
    char s[1000];
    char text[100];
    x = 10;
    for (int j = 0; j < loopText; j++) 
    {
        y = 100;
        memset(text, 0, 100);
        SAMAPP_INFO_START;
        for (int k = 0; k < j; k++)
        {
            memcpy(text, str, k+1);
            EVE_CoCmd_text(s_pHalContext, (uint16_t)x, (uint16_t)y, UNICODE_HANDLE1, 0, text);
            y += 30;
        }
        /// get fontcachequery data
        EVE_CoCmd_fontCacheQuery(s_pHalContext, &totalAfter, &usedAfter);
        sprintf_s(s, 1000, "Font cache used: total = %u, used = %d", totalAfter, usedAfter);
        EVE_CoCmd_text(s_pHalContext, s_pHalContext->Width/2, 10, 30, OPT_CENTER, s);
        SAMAPP_INFO_END;
        EVE_sleep(500);
    }

    SAMAPP_DELAY_NEXT;
#endif // EVE_SUPPORT_GEN == EVE4
}

/**
* @brief API to demonstrate unicode font
*/
void SAMAPP_Font_extendedFormat()
{
#if EVE_SUPPORT_GEN >= EVE3

    Draw_Text(s_pHalContext, "Example for: Unicode font");

    uint32_t fontAddr = RAM_G;
    //Load glyph file into BT815's flash
    //Load xfont file into graphics RAM

    // Copy font to ramg
    uint32_t fontAddrF = 4439808;
    uint32_t fontSizeF = 4544;
    EVE_CoCmd_flashRead(s_pHalContext, fontAddr, fontAddrF, fontSizeF);

    SAMAPP_INFO_START;
    EVE_CoCmd_setFont2(s_pHalContext, UNICODE_HANDLE, fontAddr, 0);
    EVE_CoCmd_text(s_pHalContext, 0, 0, UNICODE_HANDLE, 0, u8"BRT的EVE技术是一个革命性的概念，");
    EVE_CoCmd_text(s_pHalContext, 0, 30, UNICODE_HANDLE, 0, u8"利用面向对象的方法创建高质量的人机界面（HMI） 同时支持显示，");
    EVE_CoCmd_text(s_pHalContext, 0, 60, UNICODE_HANDLE, 0, u8"音频和触摸功能。");
	SAMAPP_INFO_END;
    SAMAPP_DELAY_NEXT;
#endif // BT81X
}

/**
* @brief API to demonstrate CMD_RESETFONTS
*/
void SAMAPP_Font_resetFont()
{
#if EVE_SUPPORT_GEN >= EVE3
    SAMAPP_INFO_START;
    EVE_CoCmd_resetFonts(s_pHalContext);
    SAMAPP_INFO_END;

    Draw_Text(s_pHalContext, "Example for: Font reset");

    // Copy font to ramg
    uint32_t fontAddrF = 4439808;
    uint32_t fontSizeF = 4544;
    EVE_CoCmd_flashRead(s_pHalContext, RAM_G, fontAddrF, fontSizeF);

    SAMAPP_INFO_START;
    uint32_t x = s_pHalContext->Width / 2;
    uint32_t y = s_pHalContext->Height / 2;
    EVE_CoCmd_setFont2(s_pHalContext, UNICODE_HANDLE, RAM_G, 0);
    EVE_CoCmd_text(s_pHalContext, x, y, 27, OPT_CENTER,
        "Use custom font with bitmap handle 30 from SAMAPP_ExtendedFormat_Font():");
    EVE_CoCmd_text(s_pHalContext, x, y + 50, UNICODE_HANDLE, OPT_CENTER,
        "The quick brown fox jumps over the lazy dog");
    SAMAPP_INFO_END;
    EVE_sleep(2000);

    SAMAPP_INFO_START;
    EVE_CoCmd_resetFonts(s_pHalContext);
    EVE_CoCmd_text(s_pHalContext, x, y, 27, OPT_CENTER, "Call CMD_RESETFONTS and use font 30:");
    EVE_CoCmd_text(s_pHalContext, x, y+50, UNICODE_HANDLE, OPT_CENTER,
        "The quick brown fox jumps over the lazy dog");
    SAMAPP_INFO_END;

    SAMAPP_DELAY_NEXT;
#endif // defined (BT81X_ENABLE)
}

/**
* @brief API to demonstrate custom font display
*/
void SAMAPP_Font_fromJPEG()
{
    uint8_t FontIdxTable[148];
    uint32_t fontaddr = (128 + 5 * 4);

    Draw_Text(s_pHalContext, "Example for: Custom font from a JPEG file");

    /*************************************************************************/
    /* Below code demonstrates the usage of setfont. Setfont function draws  */
    /* custom configured fonts on screen. Download the font table and raw    */
    /* font data in L1/L4/L8 format and display text                          */
    /*************************************************************************/
    memset(FontIdxTable, 16, 128);
    fontaddr = 0x00000003; //l8 format
    memcpy(&FontIdxTable[128], &fontaddr, 4);
    fontaddr = 16;//stride
    memcpy(&FontIdxTable[128 + 4], &fontaddr, 4);
    fontaddr = 16;//max width
    memcpy(&FontIdxTable[128 + 8], &fontaddr, 4);
    fontaddr = 16;//max height
    memcpy(&FontIdxTable[128 + 12], &fontaddr, 4);
    fontaddr = (1024);//data address - starts at location 1024
    memcpy(&FontIdxTable[128 + 16], &fontaddr, 4);

    EVE_Hal_wrMem(s_pHalContext, RAM_G, FontIdxTable, (128 + 5 * 4));

    /* download the jpeg image and decode */
    /* Characters from 32 to 128 are present and each character is 16*16 dimention */
    /******************* Decode jpg output into location 0 and output color format as MONO *********************/
    EVE_Util_loadImageFile(s_pHalContext, 9216, TEST_DIR "\\font16.jpg", NULL, OPT_MONO);

    SAMAPP_INFO_START;
    EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2), 20, 27, OPT_CENTER,
        "SetFont - format L8");
    EVE_CoDl_bitmapHandle(s_pHalContext, 7);
    EVE_CoDl_bitmapSource(s_pHalContext, 1024);
    EVE_CoDl_bitmapLayout(s_pHalContext, L8, 16, 16);
    EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, 16, 16);

    EVE_CoCmd_setFont(s_pHalContext, 7, 0);

    EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2), 80, 7, OPT_CENTER,
        "The quick brown fox jumps");
    EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2), 120, 7, OPT_CENTER,
        "over the lazy dog.");
    EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2), 160, 7, OPT_CENTER,
        "1234567890");

    SAMAPP_INFO_END;
    SAMAPP_DELAY_NEXT;
}

/**
* @brief API to demonstrate custom font display from a converted font file
*/
void SAMAPP_Font_fromConvertedTTF()
{
    Draw_Text(s_pHalContext, "Example for: Custom font from a converted font file");
	uint8_t head_sz = 128 + 5 * 4;
	uint8_t head[148];
	uint32_t fontaddr = (1024 + 32 * 8 * 25) - head_sz;
	EVE_Util_loadRawFile(s_pHalContext, fontaddr, TEST_DIR "\\Roboto_BoldCondensed_12.bin");
	EVE_Hal_rdMem(s_pHalContext, head, fontaddr, head_sz);
	EVE_Hal_wrMem(s_pHalContext, RAM_G, head, head_sz);

    SAMAPP_INFO_START;
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 20, 27, OPT_CENTER,
        "SetFont - format L4");
    EVE_CoDl_bitmapHandle(s_pHalContext, 6); //give index table 6
    EVE_CoDl_bitmapSource(s_pHalContext, 1024); //make the address to 0
    EVE_CoDl_bitmapLayout(s_pHalContext, L4, 8, 25); //stride is 8 and height is 25
    EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, 16, 25); //width is 16 and height is 25
    
    EVE_CoCmd_setFont(s_pHalContext, 6, 0);
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 80, 6, OPT_CENTER,
        "The quick brown fox jumps");
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 120, 6, OPT_CENTER,
        "over the lazy dog.");
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 160, 6, OPT_CENTER,
        "1234567890");
    SAMAPP_INFO_END;
    SAMAPP_DELAY_NEXT;
}

/**
* @brief Simpler method to load RAM font. 
* Use the font conversion utility to convert the desired subset of the ASCII characters, 
* load font data, and use cmd_setfont2 command.
*/
void SAMAPP_Font_fontInRAMG()
{
#if EVE_SUPPORT_GEN >= EVE3
#define FONTFILE_RAM_G_ADDRESS  1000
#define CUSTOM_RAM_FONT_HANDLE  0

    Draw_Text(s_pHalContext, "Example for: A simple method to load RAM font");
    if (!FlashHelper_SwitchFullMode(s_pHalContext))
    {
        APP_ERR("Cannot switch flash full mode");
        return;
    }
    // load Tuffy_Bold.raw in BT81X_Flash.bin
    EVE_CoCmd_flashRead(s_pHalContext, FONTFILE_RAM_G_ADDRESS, 5143616, 6656);

    SAMAPP_INFO_START;
    EVE_CoCmd_setFont2(s_pHalContext, CUSTOM_RAM_FONT_HANDLE, FONTFILE_RAM_G_ADDRESS, 32);
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2 - 30),
        (int16_t) (s_pHalContext->Height / 2 - 80), CUSTOM_RAM_FONT_HANDLE, OPT_CENTER,
        "cmd_setfont2 example");
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2 - 30),
        (int16_t) (s_pHalContext->Height / 2), CUSTOM_RAM_FONT_HANDLE, OPT_CENTER,
        "1234test  JumpingFox");
    SAMAPP_INFO_END;
    SAMAPP_DELAY_NEXT;
#endif // FT81X only
}

/**
* @brief An example for code point ordinal/UTF-8
*/
void SAMAPP_Font_indexer()
{
#if defined(BT81X_ENABLE)
    Draw_Text(s_pHalContext, "Example for: Code point ordinal/UTF-8");

    SAMAPP_INFO_START;
    helperLoadXfont(0);
    SAMAPP_INFO_END;
    EVE_sleep(2000); // display in 2 second

    // clear screen
    SAMAPP_INFO_START;
    SAMAPP_INFO_END;

    SAMAPP_INFO_START;
    helperLoadXfont(1);
    SAMAPP_INFO_END;
    SAMAPP_DELAY_NEXT;
#endif
}

void SAMAPP_Font() {
    SAMAPP_Font_romFonts();
    SAMAPP_Font_font_Cache();
    SAMAPP_Font_fontCacheQuery();
    SAMAPP_Font_extendedFormat();
    SAMAPP_Font_resetFont();
    SAMAPP_Font_fromJPEG();
    SAMAPP_Font_fromConvertedTTF();
    SAMAPP_Font_fontInRAMG();
    SAMAPP_Font_indexer();
}


