/**
 * @file Bitmap.c
 * @brief Sample usage of bitmap
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
#include "Bitmap.h"

// angle and circle                         
#define MAX_ANGLE        360
#define MAX_CIRCLE_UNIT  65536
#define SAMAPP_DELAY     EVE_sleep(2000)

/* Header of raw data containing properties of bitmap */
SAMAPP_Bitmap_header_t SAMAPP_Bitmap_RawData_Header[] =
{
    /* format,width,height,stride,arrayoffset */
    { RGB565      ,    40,      40,    40 * 2,    0 },
#if defined(FT81X_ENABLE)
    { PALETTED4444,    40,     40 ,    40    ,    0 },
    { PALETTED8   ,    480,    272,    480   ,    0 },
    { PALETTED8   ,    802,    520,    802   ,    0 },
#else
    { PALETTED    ,    40 ,    40 ,    40    ,    0 },
    { PALETTED    ,    480,    272,    480   ,    0 },
#endif
};

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void SAMAPP_Bitmap();

int main(int argc, char* argv[])
{
    s_pHalContext = &s_halContext;
    Gpu_Init(s_pHalContext);

    // read and store calibration setting
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
    Esd_Calibrate(s_pHalContext);
    Calibration_Save(s_pHalContext);
#endif

    EVE_Util_clearScreen(s_pHalContext);

    char *info[] =
    {  "EVE Sample Application",
        "This sample demonstrate the using of bitmap", 
        "",
        ""
    }; 

    while (TRUE) {
        WelcomeScreen(s_pHalContext, info);

        SAMAPP_Bitmap();

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
* @brief Draw ASTC image on screen
*
* @param str Image label
* @param x Image X
* @param y Image Y
* @param w Image W
* @param h Image H
* @param margin Image margin
* @param source Image source
* @param numcell Cell number
*/
void helperDrawASTC(const char* title, uint32_t source, uint16_t fmt, uint16_t x, uint16_t y, 
    uint16_t w, uint16_t h, uint16_t margin, uint16_t numcell)
{
#if EVE_SUPPORT_GEN >= EVE3
    int m1 = 30;

    if (title != 0 ) {
        EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 0);
        EVE_CoCmd_text(s_pHalContext, x, y, 16, 0, title);
		EVE_CoDl_colorRgb(s_pHalContext, 0xFF, 0xFF, 0xFF);
    }
    EVE_CoCmd_setBitmap(s_pHalContext, source, fmt, w, h);
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    for (int i=0; i< numcell; i++)
    {
        EVE_CoDl_cell(s_pHalContext, i);
        EVE_CoDl_vertex2f(s_pHalContext, VP(x + m1 + margin*i), VP(y));
    }
	EVE_CoDl_end(s_pHalContext);
#endif // BT81X
}

/**
* @brief Rotate image by CMD_ROTATEAROUND
*
* @param address Address of image
* @param format Image format
* @param x Image X
* @param y Image Y
* @param w Image W
* @param h Image H
* @param rotation_angle Rotate angle
*/
void helperRotateAroundOne(uint32_t address, uint32_t format, uint32_t x, uint32_t y,
    uint32_t w, uint32_t h, uint32_t rotation_angle)
{
#if (EVE_SUPPORT_GEN >= EVE3) && (defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM)) // Win32 BT81X only
    int16_t lw;
    int16_t lh;
    const uint32_t TRANSLATE_XY = 100;

    EVE_CoDl_saveContext(s_pHalContext);

    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    EVE_CoCmd_setBitmap(s_pHalContext, (int16_t) address, (int16_t) format, (int16_t) w,
        (int16_t) h);

    lw = (int16_t) (w + 2 * TRANSLATE_XY);
    lh = (int16_t) (h + 2 * TRANSLATE_XY);
    EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, lw, lh);

    EVE_CoCmd_loadIdentity(s_pHalContext);
    EVE_CoCmd_translate(s_pHalContext, TRANSLATE_XY * MAX_CIRCLE_UNIT, TRANSLATE_XY * MAX_CIRCLE_UNIT);
    EVE_CoCmd_rotateAround(s_pHalContext, w / 2, h / 2,
        rotation_angle * MAX_CIRCLE_UNIT / MAX_ANGLE, MAX_CIRCLE_UNIT);
    EVE_CoCmd_setMatrix(s_pHalContext);

    EVE_CoDl_vertex2f(s_pHalContext, VP(x), VP(y));

    EVE_CoDl_end(s_pHalContext);

    EVE_CoDl_restoreContext(s_pHalContext);
#endif // Win32 BT81X only
}

/**
* @brief Rotate image by CMD_ROTATEA and CMD_TRANSLATE
*
* @param address Address of image
* @param format Image format
* @param x Image X
* @param y Image Y
* @param w Image W
* @param h Image H
* @param rotation_angle Rotate angle
*/
void helperRotateAndTranslateOne(uint32_t address, uint32_t format, uint32_t x, uint32_t y,
    uint32_t w, uint32_t h, uint32_t rotation_angle)
{
#if (EVE_SUPPORT_GEN >= EVE3) && (defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM)) // Win32 BT81X only
    int16_t lw;
    int16_t lh;
    const uint32_t TRANSLATE_XY = 100;

    EVE_CoDl_saveContext(s_pHalContext);

    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    EVE_CoCmd_setBitmap(s_pHalContext, (int16_t) address, (int16_t) format, (int16_t) w,
        (int16_t) h);

    lw = (int16_t) (w * 2);
    lh = (int16_t) (h * 2);

    EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, lw, lh);

    EVE_CoCmd_loadIdentity(s_pHalContext);

    EVE_CoCmd_translate(s_pHalContext, (w / 2 + TRANSLATE_XY) * MAX_CIRCLE_UNIT,
        (w / 2 + TRANSLATE_XY) * MAX_CIRCLE_UNIT);
    EVE_CoCmd_rotate(s_pHalContext, rotation_angle * MAX_CIRCLE_UNIT / MAX_ANGLE);
    EVE_CoCmd_translate(s_pHalContext, -((int32_t) (w / 2)) * MAX_CIRCLE_UNIT,
        -((int32_t) (h / 2)) * MAX_CIRCLE_UNIT);

    EVE_CoCmd_setMatrix(s_pHalContext);

    EVE_CoDl_vertex2f(s_pHalContext, VP(x), VP(y));

    EVE_CoDl_end(s_pHalContext);

    EVE_CoDl_restoreContext(s_pHalContext);
#endif // Win32 BT81X only
}

/**
* @brief API to demonstrate CMD_GETIMAGE
*
*/
void SAMAPP_Bitmap_getImage()
{
#if EVE_SUPPORT_GEN == EVE4
    uint32_t source;
    uint32_t fmt;
    uint32_t w;
    uint32_t h;
    uint32_t palette;

    Draw_Text(s_pHalContext, "Example for: CMD_GETIMAGE");

    // Now try to update the allocation pointer
	Display_Start(s_pHalContext);

    EVE_Util_loadImageFile(s_pHalContext, RAM_G, TEST_DIR "/CR_003_dithering.png", NULL, OPT_RGB565);
    EVE_Cmd_waitFlush(s_pHalContext);
    EVE_CoCmd_setBitmap(s_pHalContext, RAM_G, RGB565, 800, 600);

    //Start drawing bitmap
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    EVE_CoDl_vertex2f(s_pHalContext, 0, 0);
    EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);

    EVE_sleep(500);

    // Now getImage properties
    EVE_CoCmd_getImage(s_pHalContext, &source, &fmt, &w, &h, &palette);

    printf("Loaded image: \nsource: %u, format: %u, width: %u, height: %u, palette: %u\n", 
        source, fmt, w, h, palette);
#endif // EVE_SUPPORT_GEN == EVE4
}

/**
* @brief API to demonstrate for Non Square pixel
*
*/
void SAMAPP_Bitmap_nonSquareDisplay()
{
#if EVE_SUPPORT_GEN == EVE4
#ifdef DISPLAY_RESOLUTION_WXGA
#define LCD_W 218
#define LCD_H 136
#define EXPECT_W 1280
#define EXPECT_H 800
#else
#define LCD_W 153
#define LCD_H 86
#define EXPECT_W 800
#define EXPECT_H 480
#endif

    EVE_Util_clearScreen(s_pHalContext);

    // Non-square pixel panel support in EVE4
    // EVE_Hal_wr8(s_pHalContext, REG_PCLK, 1);
	uint32_t logical_W = s_pHalContext->Height * LCD_W / LCD_H;
	// Configure panel
	EVE_Hal_wr32(s_pHalContext, REG_HSIZE, logical_W);
	EVE_CoCmd_hsf(s_pHalContext, s_pHalContext->Width);

    Draw_Text(s_pHalContext, "Example for non square pixel LCD: Draw Bitmap");
    Draw_Image(s_pHalContext, TEST_DIR "/TC_NF_24_001_862x480_862x480_RGB565_Converted.png",
        RGB565);
    EVE_sleep(2000);

    Draw_Text(s_pHalContext, "Example for non square pixel LCD: Draw primitive");
    Display_Start(s_pHalContext);
    EVE_CoDl_colorRgb(s_pHalContext, 0, 128, 0);
    Draw_Point(s_pHalContext, s_pHalContext->Height / 2, s_pHalContext->Height / 2,
        s_pHalContext->Height / 2);

    EVE_CoDl_colorRgb(s_pHalContext, 255, 128, 0);
    EVE_CoDl_begin(s_pHalContext, LINES);
    EVE_CoDl_vertex2f(s_pHalContext, 0, VP(s_pHalContext->Height / 2));
    EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Height), VP(s_pHalContext->Height / 2));
    EVE_CoCmd_text(s_pHalContext, 10, (uint16_t) (s_pHalContext->Height / 2 + 10), 30, 0, "line x");

    EVE_CoDl_begin(s_pHalContext, LINES);
    EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Height / 2), 0);
    EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Height / 2), VP(s_pHalContext->Height));
    EVE_CoCmd_text(s_pHalContext, (uint16_t) (s_pHalContext->Height / 2 + 10), 10, 30, 0, "line y");
    EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
    Display_End(s_pHalContext);
    EVE_sleep(2000);

    // Non-square pixel panel support in BT815A
    EVE_Hal_wr8(s_pHalContext, REG_PCLK, 1); /* after this display is visible on the LCD */

    // Now draw a circle
    Display_Start(s_pHalContext);
    EVE_CoDl_colorRgb(s_pHalContext, 0, 128, 0);

    uint32_t r[] = { s_pHalContext->Height * 4 / 13, s_pHalContext->Height * 2 / 13, s_pHalContext
        ->Height * 4 / 13, s_pHalContext->Height * 2 / 18, 0 };
    uint32_t y[5];
    y[0] = s_pHalContext->Height - r[0] - 10;
    y[1] = y[0] - r[1] - 10;
    y[2] = s_pHalContext->Height - r[2];
    y[3] = s_pHalContext->Height - r[2] - 10;

    uint32_t x = r[0];
    uint32_t x1;
    uint32_t y1;
    uint32_t y2;
    uint32_t distance = 0;
    for (int i = 0; i < 4; i++)
    {
        y2 = y[i];
        distance += r[i];

        if (i > 0)
            x = Math_Points_Nearby_NextX(x1, y1, y2, distance);
        Draw_Point(s_pHalContext, x, y[i], r[i]);
        x1 = x;
        y1 = y[i];
        distance = r[i];
    }
    uint32_t r5 = s_pHalContext->Height * 2 / 11;
    Draw_Point(s_pHalContext, s_pHalContext->Width - r5 - 10, r5 + 10, r5);
    Display_End(s_pHalContext);

    SAMAPP_DELAY;
#endif // EVE_SUPPORT_GEN == EVE4
}

/**
* @brief API to demonstrate CMD_LOADIMAGE
* For PNG decoder, the dithering is enabled
*
*/
void SAMAPP_Bitmap_dithering()
{
#if EVE_SUPPORT_GEN == EVE4
    uint16_t format = RGB565;
    uint16_t w = 800;
    uint16_t h = 480;
    uint16_t otp = OPT_NODL;
    uint16_t address = RAM_G;

    EVE_Util_clearScreen(s_pHalContext);


    for (int i = 0; i < 2; i++)
    {
        if (i == 0)
        {
            Draw_Text(s_pHalContext, "Example for PNG dithering: Dithering disable");
        }
        else
        {
            Draw_Text(s_pHalContext, "Example for PNG dithering: Dithering enable");

            /// Now enable dithering support
            otp |= OPT_DITHER;
        }

        EVE_Util_loadImageFile(s_pHalContext, address, TEST_DIR "\\loadimage-dither-testcase.png", NULL, otp);

        //Start drawing bitmap
        Display_Start(s_pHalContext);
        EVE_CoCmd_setBitmap(s_pHalContext, address, format, w, h);
        EVE_CoDl_begin(s_pHalContext, BITMAPS);
        EVE_CoDl_vertex2f(s_pHalContext, 0, 0);
		EVE_CoDl_end(s_pHalContext);
        Display_End(s_pHalContext);

        SAMAPP_DELAY;
    }
#endif // EVE_SUPPORT_GEN == EVE4
}

/**
* @brief API to demonstrate ASTC image on flash
*
*/
void SAMAPP_Bitmap_ASTC()
{
#if (EVE_SUPPORT_GEN >= EVE3) && (defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM))
    Draw_Text(s_pHalContext, "Example for: ASTC bitmap");

	Display_StartColor(s_pHalContext, (uint8_t[]) { 0, 0, 0 }, (uint8_t[]) { 255, 255, 255 });

    uint16_t iw = 32;
    uint16_t ih = 32;
    uint16_t format = COMPRESSED_RGBA_ASTC_4x4_KHR;

#define BITMAP_ADDRESS_ON_FLASH 5774720 //address of bitmap file from Flash map after generating Flash
    /* Switch Flash to FULL Mode */
    EVE_CoCmd_setBitmap(s_pHalContext, (0x800000 | BITMAP_ADDRESS_ON_FLASH / 32), format, iw, ih);
    //Start drawing bitmap
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    EVE_CoDl_vertex2f(s_pHalContext, 0, 0);
    EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_DELAY;
#endif // Win32 BT81X
}

/**
* @brief API to demonstrateASTC layout
*
*/
void SAMAPP_Bitmap_ASTCLayoutRAMG()
{
#if (EVE_SUPPORT_GEN >= EVE3) && (defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM))
    const char* files = TEST_DIR "\\numbers_astc12x10.raw";
    int16_t x = 0;
    int16_t y = 0;

    EVE_Util_loadRawFile(s_pHalContext, 0, files);

    Draw_Text(s_pHalContext, "Example for: ASTC bitmap on RAM_G");

    Display_Start(s_pHalContext);
    x = 20;
    y = 20;
    helperDrawASTC("1x1", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 12, 10, 20, 2);
    y += 30;
    helperDrawASTC("1x2", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 12, 20, 20, 2);
    y += 30;
    helperDrawASTC("1x3", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 12, 30, 20, 2);
    y += 40;
    helperDrawASTC("1x4", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 12, 40, 20, 2);
    y += 50;
    helperDrawASTC("1x5", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 12, 50, 20, 2);

    x += 90;
    y = 20;
    helperDrawASTC("2x1", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 24, 10, 40, 2);
    y += 30;
    helperDrawASTC("2x2", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 24, 20, 40, 2);
    y += 30;
    helperDrawASTC("2x3", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 24, 30, 40, 2);
    y += 40;
    helperDrawASTC("2x4", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 24, 40, 40, 2);
    y += 50;
    helperDrawASTC("2x5", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 24, 50, 40, 2);

    x += 110;
    y = 20;
    helperDrawASTC("3x1", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 36, 10, 50, 2);
    y += 30;
    helperDrawASTC("3x2", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 36, 20, 50, 2);
    y += 30;
    helperDrawASTC("3x3", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 36, 30, 50, 2);
    y += 40;
    helperDrawASTC("3x4", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 36, 40, 50, 2);
    y += 50;
    helperDrawASTC("3x5", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 36, 50, 50, 2);

    x += 130;
    y = 20;
    helperDrawASTC("4x1", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 48, 10, 60, 2);
    y += 30;
    helperDrawASTC("4x2", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 48, 20, 60, 2);
    helperDrawASTC("4x3", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y + 30, 48, 30, 60, 2);
    y += 70;
    helperDrawASTC("4x4", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 48, 40, 60, 2);
    y += 50;
    helperDrawASTC("4x5", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 48, 50, 60, 2);

    x += 150;
    y = 20;
    helperDrawASTC("5x1", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 60, 10, 70, 2);
    y += 30;
    helperDrawASTC("5x2", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 60, 20, 70, 2);
    y += 30;
    helperDrawASTC("5x3", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 60, 30, 70, 2);
    y += 40;
    helperDrawASTC("5x4", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 60, 40, 70, 2);
    y += 50;
    helperDrawASTC("5x5", 0, COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 60, 50, 70, 2);

    y += 70;
    EVE_CoDl_colorRgb(s_pHalContext, 255, 0, 0);
    EVE_CoCmd_text(s_pHalContext, 20, y, 28, OPT_FILL,
        "Note: Multi-celled bitmaps must have a size which is a multiple of 4 blocks");

    Display_End(s_pHalContext);
    SAMAPP_DELAY;
#endif // Win32 BT81X
}

/**
* @brief API to demonstrate ASTC layout on flash
*
*/
void SAMAPP_Bitmap_ASTCLayoutFlash()
{
#if (EVE_SUPPORT_GEN >= EVE3) && (defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM))
    char* files = TEST_DIR "\\numbers_astc12x10.raw";
    int16_t x = 0;
    int16_t y = 0;
    uint32_t astc_flash_addr = 4096;

    Draw_Text(s_pHalContext, "Example for: ASTC bitmap on Flash");

    if (0 == Ftf_Write_File_To_Flash_By_RAM_G(s_pHalContext, files, astc_flash_addr))
    {
        APP_ERR("Error when write raw file to flash");
        return;
    }

    Display_Start(s_pHalContext);
    x = 20;
    y = 20;
    helperDrawASTC("1x1", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 12, 10, 20, 1);
    y += 30;
    helperDrawASTC("1x2", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 12, 20, 20, 1);
    y += 30;
    helperDrawASTC("1x3", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 12, 30, 20, 1);
    y += 40;
    helperDrawASTC("1x4", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 12, 40, 20, 2);
    y += 50;
    helperDrawASTC("1x5", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 12, 50, 20, 1);

    x += 90;
    y = 20;
    helperDrawASTC("2x1", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 24, 10, 40, 1);
    y += 30;
    helperDrawASTC("2x2", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 24, 20, 40, 2);
    y += 30;
    helperDrawASTC("2x3", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 24, 30, 40, 1);
    y += 40;
    helperDrawASTC("2x4", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 24, 40, 40, 2);
    y += 50;
    helperDrawASTC("2x5", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 24, 50, 40, 1);

    x += 110;
    y = 20;
    helperDrawASTC("3x1", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 36, 10, 50, 1);
    y += 30;
    helperDrawASTC("3x2", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 36, 20, 50, 1);
    y += 30;
    helperDrawASTC("3x3", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 36, 30, 50, 1);
    y += 40;
    helperDrawASTC("3x4", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 36, 40, 50, 2);
    y += 50;
    helperDrawASTC("3x5", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 36, 50, 50, 1);

    x += 130;
    y = 20;
    helperDrawASTC("4x1", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 48, 10, 60, 2);
    y += 30;
    helperDrawASTC("4x2", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 48, 20, 60, 2);
    y += 30;
    helperDrawASTC("4x3", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 48, 30, 60, 2);
    y += 40;
    helperDrawASTC("4x4", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 48, 40, 60, 2);
    y += 50;
    helperDrawASTC("4x5", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 48, 50, 60, 2);

    x += 150;
    y = 20;
    helperDrawASTC("5x1", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 60, 10, 70, 1);
    y += 30;
    helperDrawASTC("5x2", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 60, 20, 70, 1);
    y += 30;
    helperDrawASTC("5x3", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 60, 30, 70, 1);
    y += 40;
    helperDrawASTC("5x4", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 60, 40, 70, 2);
    y += 50;
    helperDrawASTC("5x5", ATFLASH(astc_flash_addr), COMPRESSED_RGBA_ASTC_12x10_KHR, x, y, 60, 50, 70, 1);

    y += 70;
    EVE_CoDl_colorRgb(s_pHalContext, 255, 0, 0);
    EVE_CoCmd_text(s_pHalContext, 20, y, 28, OPT_FILL,
        "Note: Multi-celled bitmaps must have a size which is a multiple of 4 blocks");

    Display_End(s_pHalContext);
    SAMAPP_DELAY;
#endif // Win32 BT81X
}

/**
* @brief API to demonstrate ASTC layout of cell 3x2
*
*/
void SAMAPP_Bitmap_ASTCLayoutCell_1_3x2()
{
#if (EVE_SUPPORT_GEN >= EVE3) && (defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM))
    char* files[] = { TEST_DIR "\\ASTC_layout_cell_1_3x2_casei_36x240_ASTC_12x10_KHR.raw", 0 };
    const int32_t astc_flash_addr = 4096;
    int32_t source = ATFLASH(astc_flash_addr);
    int16_t x = 0;
    int16_t y = 0;

    Draw_Text(s_pHalContext, "Example for: ASTC layout of cell 3x2");

    if (0 == Ftf_Write_FileArr_To_Flash_By_RAM_G(s_pHalContext, files, astc_flash_addr))
    {
        APP_ERR("Error when write raw file to flash");
        return;
    }

    Ftf_Write_FileArr_To_RAM_G(s_pHalContext, files, 0);

    Display_Start(s_pHalContext);
    source = 0;
    x = 30;
    y = 30;
    EVE_CoCmd_fillWidth(s_pHalContext, s_pHalContext->Width - 20);
    EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 0);
    EVE_CoCmd_text(s_pHalContext, x, y, 28, OPT_FILL,
        "Display CELL 1 for 3x2 case for ASTC in RAM_G is ok:");
    EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
    EVE_CoCmd_setBitmap(s_pHalContext, source, COMPRESSED_RGBA_ASTC_12x10_KHR, 12 * 3, 10 * 2);
    EVE_CoDl_begin(s_pHalContext, BITMAPS);

    y += 55;
    EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 0);
    EVE_CoCmd_text(s_pHalContext, x + 50, y, 22, 0, "CELL 0:");
    EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
    EVE_CoDl_cell(s_pHalContext, 0); //#CELL 0
    EVE_CoDl_vertex2f(s_pHalContext, VP(x + 50 + 70), VP(y - 2));

    x += 200;
	EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 0);
    EVE_CoCmd_text(s_pHalContext, x, y, 22, 0, "CELL 1:");
	EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
	EVE_CoDl_cell(s_pHalContext, 1); //#CELL 1
	EVE_CoDl_vertex2f(s_pHalContext, VP(x + 70), VP(y - 2));
	EVE_CoDl_end(s_pHalContext);

    x = 30;
    y += 80;
    source = ATFLASH(astc_flash_addr);
    EVE_CoCmd_fillWidth(s_pHalContext, s_pHalContext->Width - 20);
	EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 0);
    EVE_CoCmd_text(s_pHalContext, x, y, 28, OPT_FILL,
        "Display CELL 1 for 3x2 case for ASTC in Flash shall show some artifacts:");
	EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
    EVE_CoCmd_setBitmap(s_pHalContext, source, COMPRESSED_RGBA_ASTC_12x10_KHR, 12 * 3, 10 * 2);
    EVE_CoDl_begin(s_pHalContext, BITMAPS);

    y += 50;
	EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 0);
    EVE_CoCmd_text(s_pHalContext, x + 50, y, 22, 0, "CELL 0:");
	EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
	EVE_CoDl_cell(s_pHalContext, 0); //#CELL 0
	EVE_CoDl_vertex2f(s_pHalContext, VP(x + 50 + 70), VP(y - 2));

    x += 150;
	EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 0);
    EVE_CoCmd_text(s_pHalContext, x + 50, y, 22, 0, "CELL 1:");
	EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
	EVE_CoDl_cell(s_pHalContext, 1); //#CELL 1
	EVE_CoDl_vertex2f(s_pHalContext, VP(x + 50 + 70), VP(y - 2));
	EVE_CoDl_end(s_pHalContext);

    y += 70;
	EVE_CoDl_colorRgb(s_pHalContext, 255, 0, 0);
    EVE_CoCmd_text(s_pHalContext, 30, y, 28, OPT_FILL,
        "Note: Multi-celled bitmaps must have a size which is a multiple of 4 blocks");

    Display_End(s_pHalContext);
    SAMAPP_DELAY;
#endif // Win32 BT81X
}

/**
* @brief API to demonstrate multicell ASTC bitmap on RAMG
* 
* Each cell of the multi-cell ASTC bitmap shall be multiple of 4 blocks in height. 
* For example, if one bitmap is in 100 pixel x 672 pixel (width x height), the legal cell size is 100x192, 100x96.
* The illegal cell size shall be 100x84 (8 cells), 100x72 (10 cells) etc...
*/
void SAMAPP_Bitmap_ASTCMultiCellRAMG()
{
#if EVE_SUPPORT_GEN >= EVE3
    const char* astc_file = TEST_DIR "\\cell_100x672_COMPRESSED_RGBA_ASTC_4x4_KHR.raw";
    int16_t x = 0;
    int16_t y = 20;
    uint32_t astc_addr = 0;

    Draw_Text(s_pHalContext, "Example for: Multicell ASTC bitmap on RAM_G");

	EVE_Util_loadRawFile(s_pHalContext, astc_addr, astc_file);

    Display_Start(s_pHalContext);
    EVE_CoDl_colorRgb(s_pHalContext, 0x33, 0x33, 0x33);
    EVE_CoCmd_text(s_pHalContext, 20, y, 28, 0,
        "Each cell of the multi-cell ASTC bitmap shall be multiple of 4 blocks in height.\n\n"
        "For example, if one bitmap is in 100x672: The legal cell size is 100x192, 100x96. \n"
        "The illegal cell size shall be 100x84 (8 cells), 100x72 (10 cells) etc..."
        "\n\n"
        "Image of 100x96 (7 cells, 600 blocks / cell):\n");

    y += 180;
	EVE_CoDl_colorRgb(s_pHalContext, 0xff, 0xff, 0xff);
    helperDrawASTC(0, astc_addr, COMPRESSED_RGBA_ASTC_4x4_KHR, x, y, 100, 96, 105, 7);

    y += 140;
	EVE_CoDl_colorRgb(s_pHalContext, 0x33, 0x33, 0x33);
    EVE_CoCmd_text(s_pHalContext, 20, y, 28, OPT_FILL,
        "Image of 100x84 (8 cells, 525 blocks / cell):\n");
    y += 40;
	EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
    helperDrawASTC(0, astc_addr, COMPRESSED_RGBA_ASTC_4x4_KHR, x, y, 100, 84, 105, 7);

    Display_End(s_pHalContext);
    SAMAPP_DELAY;
#endif
}

/**
* @brief API to demonstrate multicell ASTC bitmap on Flash
*
* Each cell of the multi-cell ASTC bitmap shall be multiple of 4 blocks in height.
* For example, if one bitmap is in 100 pixel x 672 pixel (width x height), the legal cell size is 100x192, 100x96.
* The illegal cell size shall be 100x84 (8 cells), 100x72 (10 cells) etc...
*/
void SAMAPP_Bitmap_ASTCMultiCellFlash()
{
#if EVE_SUPPORT_GEN >= EVE3
    const char* astc_file = TEST_DIR "\\cell_100x672_COMPRESSED_RGBA_ASTC_4x4_KHR.raw";
    int16_t x = 0;
    int16_t y = 20;
    uint32_t astc_addr = 4096;
    uint8_t save_adaptive = EVE_Hal_rd8(s_pHalContext, REG_ADAPTIVE_FRAMERATE);

    Draw_Text(s_pHalContext, "Example for: Multicell ASTC bitmap on Flash");

    if (0 == Ftf_Write_File_To_Flash_By_RAM_G(s_pHalContext, astc_file, astc_addr))
	{
		APP_ERR("Error when write raw file to flash");
		return;
	}
    // On large screen, enable REG_ADAPTIVE_FRAMERATE so image can displayed
    if (s_pHalContext->Width >= 800) {
        EVE_Hal_wr8(s_pHalContext, REG_ADAPTIVE_FRAMERATE, 1);
    }

    Display_Start(s_pHalContext);
	EVE_CoDl_colorRgb(s_pHalContext, 0x33, 0x33, 0x33);
    EVE_CoCmd_fillWidth(s_pHalContext, s_pHalContext->Width - 20);
    EVE_CoCmd_text(s_pHalContext, 20, y, 28, 0,
        "Each cell of the multi-cell ASTC bitmap shall be multiple of 4 blocks in height.\n\n"
        "For example, if one bitmap is in 100x672: The legal cell size is 100x192, 100x96. \n"
        "The illegal cell size shall be 100x84 (8 cells), 100x72 (10 cells) etc..."
        "\n\n"
        "Image of 100x96 (7 cells, 600 blocks / cell):\n");

    y += 180;
	EVE_CoDl_colorRgb(s_pHalContext, 0xff, 0xff, 0xff);
    helperDrawASTC(0, ATFLASH(astc_addr), COMPRESSED_RGBA_ASTC_4x4_KHR, x, y, 100, 96, 105, 7);

    y += 140;
	EVE_CoDl_colorRgb(s_pHalContext, 0x33, 0x33, 0x33);
    EVE_CoCmd_text(s_pHalContext, 20, y, 28, OPT_FILL,
        "Image of 100x84 (8 cells, 525 blocks / cell):\n");
    y += 40;
	EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
    helperDrawASTC(0, ATFLASH(astc_addr), COMPRESSED_RGBA_ASTC_4x4_KHR, x, y, 100, 84, 105, 7);
    Display_End(s_pHalContext);
    SAMAPP_DELAY;

    if (s_pHalContext->Width >= 800) {
        EVE_Hal_wr8(s_pHalContext, REG_ADAPTIVE_FRAMERATE, save_adaptive);
    }
#endif
}

/**
* @brief API to demonstrate image rotate
*
*/
void SAMAPP_Bitmap_rotate()
{
#if (EVE_SUPPORT_GEN >= EVE3) && (defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM))
	uint16_t image_w = 256;
	uint16_t image_h = 256;

    Draw_Text(s_pHalContext, "Example for: Bitmap rotate");
	EVE_Util_loadImageFile(s_pHalContext, 0, TEST_DIR "\\mandrill256.jpg", NULL, OPT_RGB565);

    for (uint16_t i = 0; i <= 360; i++)
    {
		Display_Start(s_pHalContext);

        EVE_CoDl_begin(s_pHalContext, BITMAPS);
		EVE_CoCmd_setBitmap(s_pHalContext, RAM_G, RGB565, image_w, image_h);
        EVE_CoCmd_loadIdentity(s_pHalContext);
        EVE_CoCmd_rotateAround(s_pHalContext, image_w / 2, image_h / 2, i * 65536 / 360, 65536 * 1);
        EVE_CoCmd_setMatrix(s_pHalContext);
        EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - image_w / 2), VP(s_pHalContext->Height / 2 - image_h / 2));
        EVE_CoDl_end(s_pHalContext);

        EVE_CoCmd_loadIdentity(s_pHalContext);
        EVE_CoCmd_setMatrix(s_pHalContext);
		
		EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 0);
		EVE_CoCmd_text(s_pHalContext, 10, 0, 24, 0, "CMD_ROTATEAROUND"); //text info

        Display_End(s_pHalContext);
        EVE_sleep(5);
    }
    SAMAPP_DELAY;
#endif // Win32 BT81X
}

/**
* @brief Draw set 12
*
*/
void SAMAPP_Bitmap_rotateAndTranslate()
{
#if (EVE_SUPPORT_GEN >= EVE3) && (defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM))
    int count = 0;
    const uint32_t TRANSLATE_XY = 100;

    // tiles
    uint16_t tile_w = 256;
    uint16_t tile_h = 256;
    uint16_t tile1_x = -30;
    uint16_t tile1_y = 50;
    uint16_t tile2_x = (uint16_t)(s_pHalContext->Width - tile1_x - tile_w - 200);
    uint16_t tile2_y = tile1_y;

    Draw_Text(s_pHalContext, "Example for: Bitmap rotate and translate");
	EVE_Util_loadImageFile(s_pHalContext, 0, TEST_DIR "\\mandrill256.jpg", NULL, OPT_RGB565);

    while (count++ < 60 * 10)
    { // wait 10 seconds, 60 FPS
      /*Display List start*/
		Display_StartColor(s_pHalContext, (uint8_t[]) { 0, 0, 0 }, (uint8_t[]) { 255, 255, 255 });

        static uint16_t rotation_angle = 0;
        static uint16_t font_size = 29;
        static uint16_t option = 0;

        rotation_angle = (rotation_angle + 1) % MAX_ANGLE;

        helperRotateAroundOne(RAM_G, RGB565, tile1_x, tile1_y, tile_w, tile_h, rotation_angle);
        helperRotateAndTranslateOne(RAM_G, RGB565, tile2_x, tile2_y, tile_w, tile_h, rotation_angle);

        EVE_CoCmd_text(s_pHalContext, tile1_x + TRANSLATE_XY / 2, tile1_y, font_size, option, "Rotate by RotateAround");
        EVE_CoCmd_text(s_pHalContext, tile2_x + TRANSLATE_XY / 2, tile2_y, font_size, option, "Rotate by Rotate and Translate");

        Display_End(s_pHalContext);
    }
    SAMAPP_DELAY;
#endif
}

/**
* @brief demonstrates the usage of loadimage function
* Download the jpg data into command buffer and in turn coprocessor decodes and dumps into location 0 with L8 format
*
*/
void SAMAPP_Bitmap_loadImageMono()
{
#if EVE_SUPPORT_GEN >= EVE2
	int16_t ImgW = 256;
	int16_t ImgH = 256;
	const char *file = TEST_DIR "\\mandrill256.jpg";

	Draw_Text(s_pHalContext, "Example for: Load image and display as monochromic image");

	EVE_Util_loadImageFile(s_pHalContext, 0, file, NULL, OPT_MONO);

	// Decode jpg output into location 0 and output color format as L8
	Display_Start(s_pHalContext);

	EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_blendFunc(s_pHalContext, SRC_ALPHA, ZERO);
	EVE_CoCmd_setBitmap(s_pHalContext, 0, L8, ImgW, ImgH);
	EVE_CoDl_vertex2f(s_pHalContext, VP((s_pHalContext->Width - ImgW) / 2), VP((s_pHalContext->Height - ImgH) / 2));
	EVE_CoDl_blendFunc_default(s_pHalContext);
	EVE_CoDl_end(s_pHalContext);

	EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 255);
	EVE_CoCmd_text(s_pHalContext, (s_pHalContext->Width) / 2, (s_pHalContext->Height) / 2, 26, OPT_CENTER, "Display bitmap by jpg decode L8");

	Display_End(s_pHalContext);
	SAMAPP_DELAY;
#endif
}

/**
* @brief Load image to Coprocessor full color
*
*/
void SAMAPP_Bitmap_loadImageFullColor()
{
#if EVE_SUPPORT_GEN >= EVE2
	int16_t ImgW = 256;
	int16_t ImgH = 256;
    const char* file = TEST_DIR "\\mandrill256.jpg";

	Draw_Text(s_pHalContext, "Example for: Load image and display as full color image");

    /// Decode jpg output into location 0 and output color format as L8
    Display_Start(s_pHalContext);
	EVE_Util_loadImageFile(s_pHalContext, 0, file, NULL, OPT_RGB565);

    EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_vertex2f(s_pHalContext, VP((s_pHalContext->Width - ImgW) / 2), VP((s_pHalContext->Height - ImgH) / 2));
    EVE_CoDl_end(s_pHalContext);

	EVE_CoCmd_text(s_pHalContext, (s_pHalContext->Width) / 2, (s_pHalContext->Height) / 2, 26, OPT_CENTER, "Display bitmap by jpg decode");

    Display_End(s_pHalContext);
    SAMAPP_DELAY;
#endif
}

/**
* @brief Load DXT1 compressed image. The BRIDGETEK DXT1 conversion utility outputs 2 seperate files.
* The 2 files should be combined to create the final image.  The bitmap size can be reduced up to 4 folds of the original size.
*
*/
void SAMAPP_Bitmap_DXT1()
{
    //RAM_G is starting address in graphics RAM, for example 00 0000h
    uint16_t imgWidth = 320;
    uint16_t imgHeight = 240;
	uint16_t colorLayer_width = imgWidth / 4;
	uint16_t colorLayer_height = imgHeight / 4;
	uint16_t colorLayer_stride = colorLayer_width * 2;
    uint16_t gradientLayer_width = imgWidth;
	uint16_t gradientLayer_height = imgHeight;
	uint16_t gradientLayer_stride = gradientLayer_width / 8;
	uint16_t szPerFile = colorLayer_stride * colorLayer_height * 2 ;
	uint8_t colorHandle = 1;
	uint8_t gradientHandle = 2;

	if (gradientLayer_width % 8 != 0)
		gradientLayer_stride += 1;

    Draw_Text(s_pHalContext, "Example for: Load DXT1_L1_RGB565 compressed image");

	EVE_Util_loadRawFile(s_pHalContext, RAM_G, TEST_DIR "\\bird_320x240_RGB565.raw");
	EVE_Util_loadRawFile(s_pHalContext, RAM_G + szPerFile, TEST_DIR "\\bird_320x240_L1.raw");

    Display_Start(s_pHalContext);

    EVE_CoCmd_loadIdentity(s_pHalContext);
    EVE_CoCmd_setMatrix(s_pHalContext);

    EVE_CoDl_saveContext(s_pHalContext);

	// color handle
	EVE_CoDl_bitmapHandle(s_pHalContext, colorHandle);
#if EVE_SUPPORT_GEN >= EVE2
	EVE_CoCmd_setBitmap(s_pHalContext, RAM_G, RGB565, colorLayer_width, colorLayer_height);
#else
	EVE_CoDl_bitmapSource(s_pHalContext, RAM_G);
	EVE_CoDl_bitmapLayout(s_pHalContext, RGB565, colorLayer_stride, colorLayer_height);
#endif
	EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, imgWidth, imgHeight);

    // gradient handle
	EVE_CoDl_bitmapHandle(s_pHalContext, gradientHandle);
#if EVE_SUPPORT_GEN >= EVE2
	EVE_CoCmd_setBitmap(s_pHalContext, RAM_G + szPerFile, L1, imgWidth, imgHeight);
#else
	EVE_CoDl_bitmapSource(s_pHalContext, RAM_G + szPerFile);
	EVE_CoDl_bitmapLayout(s_pHalContext, L1, imgWidth / 8, imgHeight);
#endif
                                                                                           
    // start drawing bitmaps
	EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_blendFunc(s_pHalContext, ONE, ZERO);
	EVE_CoDl_colorA(s_pHalContext, 0x55);
	EVE_CoDl_bitmapHandle(s_pHalContext, gradientHandle);
	EVE_CoDl_cell(s_pHalContext, 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2));
	EVE_CoDl_blendFunc(s_pHalContext, ONE, ONE);
	EVE_CoDl_colorA(s_pHalContext, 0xAA);
	EVE_CoDl_cell(s_pHalContext, 1);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2));

	EVE_CoDl_colorMask(s_pHalContext, 1, 1, 1, 0);
	EVE_CoCmd_scale(s_pHalContext, 4UL * 65536UL, 4UL * 65536UL); // Color passes, scaled up 4x, nearest
	EVE_CoCmd_setMatrix(s_pHalContext);
	EVE_CoDl_blendFunc(s_pHalContext, DST_ALPHA, ZERO);
	EVE_CoDl_bitmapHandle(s_pHalContext, colorHandle);
	EVE_CoDl_cell(s_pHalContext, 1);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2)); // Color layer 1 (RGB*L2)
	EVE_CoDl_blendFunc(s_pHalContext, ONE_MINUS_DST_ALPHA, ONE);
	EVE_CoDl_cell(s_pHalContext, 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2)); // Color layer 0 (RGB*(1-L2) + DST)
	EVE_CoDl_end(s_pHalContext);
	EVE_CoDl_restoreContext(s_pHalContext);

	// reset the transformation matrix because its not part of the context, RESTORE_CONTEXT() command will not revert the command.
	EVE_CoCmd_loadIdentity(s_pHalContext);
	EVE_CoCmd_setMatrix(s_pHalContext);

	EVE_CoDl_colorRgb(s_pHalContext, 255, 0, 0);
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 50, 31, OPT_CENTER,
        "DXT1_L1_RGB565: 37.5KB.");
    EVE_CoCmd_text(s_pHalContext, (int16_t) (s_pHalContext->Width / 2), 80, 31, OPT_CENTER,
        "RGB565: 150KB.");

    Display_End(s_pHalContext);
    SAMAPP_DELAY;
}

/**
 * @brief Load DXT1L2 compressed image. The BRIDGETEK DXT1L2 conversion utility outputs 2 seperate files
 * The 2 files should be combined to create the final image.  The bitmap size can be reduced up to 4 folds of the original size.
 *
 */
void SAMAPP_Bitmap_DXT1L2()
{
#if EVE_SUPPORT_GEN >= EVE2
	// RAM_G is starting address in graphics RAM, for example 00 0000h
	uint16_t imgWidth = 320;
	uint16_t imgHeight = 240;
	uint16_t colorLayer_width = imgWidth / 4;
	uint16_t colorLayer_height = imgHeight / 4;
	uint16_t colorLayer_stride = colorLayer_width * 2;
	uint16_t cFileSize = colorLayer_stride * colorLayer_height * 2;
	uint8_t colorHandle = 1;
	uint8_t gradientHandle = 2;

	Draw_Text(s_pHalContext, "Example for: Load DXT1_L2_RGB565 compressed image");

	EVE_Util_loadRawFile(s_pHalContext, RAM_G, TEST_DIR "\\bird_320x240_RGB565.raw");
	EVE_Util_loadRawFile(s_pHalContext, RAM_G + cFileSize, TEST_DIR "\\bird_320x240_L2.raw");

	Display_Start(s_pHalContext);

	EVE_CoCmd_loadIdentity(s_pHalContext);
	EVE_CoCmd_setMatrix(s_pHalContext);

	EVE_CoDl_saveContext(s_pHalContext);

    // color handle
	EVE_CoDl_bitmapHandle(s_pHalContext, colorHandle);
	EVE_CoCmd_setBitmap(s_pHalContext, RAM_G, RGB565, colorLayer_width, colorLayer_height);
	EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, imgWidth, imgHeight);

	// gradient handle
	EVE_CoDl_bitmapHandle(s_pHalContext, gradientHandle);
	EVE_CoCmd_setBitmap(s_pHalContext, RAM_G + cFileSize, L2, imgWidth, imgHeight);

	// start drawing bitmaps
	EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_blendFunc(s_pHalContext, ONE, ZERO);
	EVE_CoDl_colorA(s_pHalContext, 0xFF);
	EVE_CoDl_bitmapHandle(s_pHalContext, gradientHandle);
	EVE_CoDl_cell(s_pHalContext, 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2));

	EVE_CoDl_colorMask(s_pHalContext, 1, 1, 1, 0);
	EVE_CoCmd_scale(s_pHalContext, 4UL * 65536UL, 4UL * 65536UL); // Color passes, scaled up 4x, nearest
	EVE_CoCmd_setMatrix(s_pHalContext);
	EVE_CoDl_blendFunc(s_pHalContext, DST_ALPHA, ZERO);
	EVE_CoDl_bitmapHandle(s_pHalContext, colorHandle);
	EVE_CoDl_cell(s_pHalContext, 1);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2)); // Color layer 1 (RGB*L2)
	EVE_CoDl_blendFunc(s_pHalContext, ONE_MINUS_DST_ALPHA, ONE);
	EVE_CoDl_cell(s_pHalContext, 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2)); // Color layer 0 (RGB*(1-L2) + DST)
	EVE_CoDl_end(s_pHalContext);
	EVE_CoDl_restoreContext(s_pHalContext);

	// reset the transformation matrix because its not part of the context, RESTORE_CONTEXT() command will not revert the command.
	EVE_CoCmd_loadIdentity(s_pHalContext);
	EVE_CoCmd_setMatrix(s_pHalContext);

	EVE_CoDl_colorRgb(s_pHalContext, 255, 0, 0);
	EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2), 50, 31, OPT_CENTER,
	    "DXT1_L2_RGB565: 37.5KB.");
	EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2), 80, 31, OPT_CENTER,
	    "RGB565: 150KB.");

	Display_End(s_pHalContext);
	SAMAPP_DELAY;
#endif
}

/**
 * @brief Load DXT1PALETTED compressed image. The BRIDGETEK DXT1PALETTED conversion utility outputs 4 seperate files: b0, b1, lut, index
 * The 4 files should be combined to create the final image.  The bitmap size can be reduced up to 5 folds of the original size.
 *
 */
void SAMAPP_Bitmap_DXT1PALETTED()
{
#if EVE_SUPPORT_GEN >= EVE2
	// RAM_G is starting address in graphics RAM, for example 00 0000h
	uint16_t imgWidth = 320;
	uint16_t imgHeight = 240;
	uint16_t palWidth = imgWidth / 4;
	uint16_t palHeight = imgHeight / 4;
	uint16_t fileSize = (imgWidth / 4) * (imgHeight / 4) * 2;
	uint16_t paletteAddr = RAM_G;
	uint16_t colorAddr = paletteAddr + 512;
	uint16_t gradientAddr = colorAddr + fileSize;
	uint8_t colorHandle = 1;
	uint8_t gradientHandle = 2;

	Draw_Text(s_pHalContext, "Example for: Load DXT1_L1_Paletted565 compressed image");

	EVE_Util_loadRawFile(s_pHalContext, paletteAddr, TEST_DIR "\\bird_320x240_lut.raw");
	EVE_Util_loadRawFile(s_pHalContext, colorAddr, TEST_DIR "\\bird_320x240_index.raw");
	EVE_Util_loadRawFile(s_pHalContext, gradientAddr, TEST_DIR "\\bird_320x240_L1.raw");

	Display_Start(s_pHalContext);

	EVE_CoCmd_loadIdentity(s_pHalContext);
	EVE_CoCmd_setMatrix(s_pHalContext);

	EVE_CoDl_saveContext(s_pHalContext);

	// paletted handle
	EVE_CoDl_bitmapHandle(s_pHalContext, colorHandle);
	EVE_CoCmd_setBitmap(s_pHalContext, colorAddr, PALETTED565, palWidth, palHeight);
	EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, imgWidth, imgHeight);
	EVE_CoDl_paletteSource(s_pHalContext, paletteAddr);

	// L1 handle
	EVE_CoDl_bitmapHandle(s_pHalContext, gradientHandle);
	EVE_CoCmd_setBitmap(s_pHalContext, gradientAddr, L1, imgWidth, imgHeight);

	// start drawing bitmaps
	EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_blendFunc(s_pHalContext, ONE, ZERO);
	EVE_CoDl_colorA(s_pHalContext, 0x55);
	EVE_CoDl_bitmapHandle(s_pHalContext, gradientHandle);
	EVE_CoDl_cell(s_pHalContext, 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2));
	EVE_CoDl_blendFunc(s_pHalContext, ONE, ONE);
	EVE_CoDl_colorA(s_pHalContext, 0xAA);
	EVE_CoDl_cell(s_pHalContext, 1);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2));

	EVE_CoDl_colorMask(s_pHalContext, 1, 1, 1, 0);
	EVE_CoCmd_scale(s_pHalContext, 4UL * 65536UL, 4UL * 65536UL); // Color passes, scaled up 4x, nearest
	EVE_CoCmd_setMatrix(s_pHalContext);
	EVE_CoDl_blendFunc(s_pHalContext, DST_ALPHA, ZERO);
	EVE_CoDl_bitmapHandle(s_pHalContext, colorHandle);
	EVE_CoDl_cell(s_pHalContext, 1);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2)); // Color layer 1 (RGB*L2)
	EVE_CoDl_blendFunc(s_pHalContext, ONE_MINUS_DST_ALPHA, ONE);
	EVE_CoDl_cell(s_pHalContext, 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2)); // Color layer 0 (RGB*(1-L2) + DST)
	EVE_CoDl_end(s_pHalContext);
	EVE_CoDl_restoreContext(s_pHalContext);

	// reset the transformation matrix because its not part of the context, RESTORE_CONTEXT() command will not revert the command.
	EVE_CoCmd_loadIdentity(s_pHalContext);
	EVE_CoCmd_setMatrix(s_pHalContext);

	EVE_CoCmd_dl(s_pHalContext, COLOR_RGB(255, 0, 0));
	EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2), 50, 31, OPT_CENTER,
	    "DXT1_L1_Paletted565: 28.6KB.");
	EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2), 80, 31, OPT_CENTER,
	    "RGB565: 150KB.");

	Display_End(s_pHalContext);
	SAMAPP_DELAY;
#endif
}

/**
 * @brief Load DXT1L2PALETTED compressed image. The BRIDGETEK DXT1L2PALETTED conversion utility outputs 3 seperate files: L2, lut, index
 * The 3 files should be combined to create the final image.  The bitmap size can be reduced up to 5 folds of the original size.
 *
 */
void SAMAPP_Bitmap_DXT1L2PALETTED()
{
#if EVE_SUPPORT_GEN >= EVE2
	// RAM_G is starting address in graphics RAM, for example 00 0000h
	uint16_t imgWidth = 320;
	uint16_t imgHeight = 240;
	uint16_t palWidth = imgWidth / 4;
	uint16_t palHeight = imgHeight / 4;
	uint16_t idxFileSize = (imgWidth / 4) * (imgHeight / 4) * 2;
	uint16_t paletteAddr = RAM_G;
	uint16_t colorAddr = paletteAddr + 512;
	uint16_t gradientAddr = colorAddr + idxFileSize;
	uint8_t colorHandle = 1;
	uint8_t gradientHandle = 2;

	Draw_Text(s_pHalContext, "Example for: Load DXT1_L2_Paletted565 compressed image");

	EVE_Util_loadRawFile(s_pHalContext, paletteAddr, TEST_DIR "\\bird_320x240_lut.raw");
	EVE_Util_loadRawFile(s_pHalContext, colorAddr, TEST_DIR "\\bird_320x240_index.raw");
	EVE_Util_loadRawFile(s_pHalContext, gradientAddr, TEST_DIR "\\bird_320x240_L2.raw");

	Display_Start(s_pHalContext);

	EVE_CoCmd_loadIdentity(s_pHalContext);
	EVE_CoCmd_setMatrix(s_pHalContext);

	EVE_CoDl_saveContext(s_pHalContext);

	// paletted handle
	EVE_CoDl_bitmapHandle(s_pHalContext, colorHandle);
	EVE_CoCmd_setBitmap(s_pHalContext, colorAddr, PALETTED565, palWidth, palHeight);
	EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, imgWidth, imgHeight);
	EVE_CoDl_paletteSource(s_pHalContext, paletteAddr);

	// L2 handle
	EVE_CoDl_bitmapHandle(s_pHalContext, gradientHandle);
	EVE_CoCmd_setBitmap(s_pHalContext, gradientAddr, L2, imgWidth, imgHeight);

	// start drawing bitmaps
	EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_blendFunc(s_pHalContext, ONE, ZERO);
	EVE_CoDl_colorA(s_pHalContext, 0xFF);
	EVE_CoDl_bitmapHandle(s_pHalContext, gradientHandle);
	EVE_CoDl_cell(s_pHalContext, 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2));

	EVE_CoDl_colorMask(s_pHalContext, 1, 1, 1, 0);
	EVE_CoCmd_scale(s_pHalContext, 4UL * 65536UL, 4UL * 65536UL); // Color passes, scaled up 4x, nearest
	EVE_CoCmd_setMatrix(s_pHalContext);
	EVE_CoDl_blendFunc(s_pHalContext, DST_ALPHA, ZERO);
	EVE_CoDl_bitmapHandle(s_pHalContext, colorHandle);
	EVE_CoDl_cell(s_pHalContext, 1);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2)); // Color layer 1 (RGB*L2)
	EVE_CoDl_blendFunc(s_pHalContext, ONE_MINUS_DST_ALPHA, ONE);
	EVE_CoDl_cell(s_pHalContext, 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - imgWidth / 2), VP(s_pHalContext->Height / 2 - imgHeight / 2)); // Color layer 0 (RGB*(1-L2) + DST)
	EVE_CoDl_end(s_pHalContext);
	EVE_CoDl_restoreContext(s_pHalContext);

	// reset the transformation matrix because its not part of the context, RESTORE_CONTEXT() command will not revert the command.
	EVE_CoCmd_loadIdentity(s_pHalContext);
	EVE_CoCmd_setMatrix(s_pHalContext);

	EVE_CoDl_colorRgb(s_pHalContext, 255, 0, 0);
	EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2), 50, 31, OPT_CENTER,
	    "DXT1_L2_Paletted565: 28.6KB.");
	EVE_CoCmd_text(s_pHalContext, (int16_t)(s_pHalContext->Width / 2), 80, 31, OPT_CENTER,
	    "Original: 150KB.");

	Display_End(s_pHalContext);
	SAMAPP_DELAY;
#endif
}

/**
* @brief API to demonstrate Paletted8 format
*
*/
void SAMAPP_Bitmap_paletted8()
{
#if EVE_SUPPORT_GEN >= EVE2
    const SAMAPP_Bitmap_header_t* p_bmhdr;
    int32_t pal_mem_addr = 900 * 1024;

    Draw_Text(s_pHalContext, "Example for: Paletted8 format");

	EVE_Util_loadRawFile(s_pHalContext, RAM_G, TEST_DIR "\\Background_index.raw");
	EVE_Util_loadRawFile(s_pHalContext, pal_mem_addr, TEST_DIR "\\Background_lut.raw");

    p_bmhdr = &SAMAPP_Bitmap_RawData_Header[3];
	Display_Start(s_pHalContext);
    EVE_CoDl_bitmapSource(s_pHalContext, RAM_G);
	EVE_CoDl_bitmapLayout(s_pHalContext, p_bmhdr->Format, p_bmhdr->Stride, p_bmhdr->Height);
	EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, p_bmhdr->Width, p_bmhdr->Height);

    EVE_CoDl_begin(s_pHalContext, BITMAPS); // start drawing bitmaps

    EVE_CoDl_blendFunc(s_pHalContext, ONE, ZERO);
	EVE_CoDl_colorMask(s_pHalContext, 0, 0, 0, 1);
	EVE_CoDl_paletteSource(s_pHalContext, pal_mem_addr + 3);
	EVE_CoDl_vertex2f(s_pHalContext, 0, 0);

	EVE_CoDl_blendFunc(s_pHalContext, DST_ALPHA, ONE_MINUS_DST_ALPHA);
	EVE_CoDl_colorMask(s_pHalContext, 1, 0, 0, 0);
	EVE_CoDl_paletteSource(s_pHalContext, pal_mem_addr + 2);
	EVE_CoDl_vertex2f(s_pHalContext, 0, 0);

	EVE_CoDl_colorMask(s_pHalContext, 0, 1, 0, 0);
	EVE_CoDl_paletteSource(s_pHalContext, pal_mem_addr + 1);
	EVE_CoDl_vertex2f(s_pHalContext, 0, 0);

    EVE_CoDl_colorMask(s_pHalContext, 0, 0, 1, 0);
	EVE_CoDl_paletteSource(s_pHalContext, pal_mem_addr + 0);
	EVE_CoDl_vertex2f(s_pHalContext, 0, 0);

	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_DELAY;
#endif
}

/**
* @brief this function demonstrates the usage of the paletted bitmap converted by the BRIDGETEK palette converter
*
*/
void SAMAPP_Bitmap_paletted4444()
{
#if EVE_SUPPORT_GEN >= EVE2
	uint16_t bitmapHeight = 128;
	uint16_t bitmapWidth = 128;

	Draw_Text(s_pHalContext, "Example for: Paletted4444 format");

	EVE_Util_loadRawFile(s_pHalContext, RAM_G, TEST_DIR "\\Tomato_lut.raw");
	EVE_Util_loadRawFile(s_pHalContext, 1024, TEST_DIR "\\Tomato_index.raw");

	Display_Start(s_pHalContext);
	EVE_CoCmd_setBitmap(s_pHalContext, 1024, PALETTED4444, bitmapWidth, bitmapHeight);
	EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, bitmapWidth, bitmapHeight);
	EVE_CoDl_paletteSource(s_pHalContext, RAM_G);

	EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2 - bitmapWidth / 2), VP(s_pHalContext->Height / 2 - bitmapHeight / 2));
	EVE_CoDl_end(s_pHalContext);

	Display_End(s_pHalContext);
	SAMAPP_DELAY;
#endif
}

/**
* @brief support bitmap resolutions up to 2048x2048
* FT80x can only display bitmaps no larger than 512x512, FT81x
* If the bitmap dimensions are bigger than 512 in either direction
*/
void SAMAPP_Bitmap_higherResolutionBitmap()
{
#if EVE_SUPPORT_GEN >= EVE2
    Draw_Text(s_pHalContext, "Example for: Bitmap resolutions up to 2048x2048");
	Display_Start(s_pHalContext);

    uint16_t iw = 800;
    uint16_t ih = 480;
    uint16_t format = RGB565;

    //load bitmap file into graphics RAM
    //RAM_G is starting address in graphics RAM, for example 00 0000h
	EVE_Util_loadRawFile(s_pHalContext, RAM_G, TEST_DIR "\\flower_800x480_800x480_RGB565.raw");

    //Start drawing bitmap
    EVE_CoDl_saveContext(s_pHalContext);
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_bitmapHandle(s_pHalContext, 1);
	EVE_CoCmd_setBitmap(s_pHalContext, RAM_G, format, iw, ih);
    EVE_CoDl_vertex2f(s_pHalContext, 0, 0);
    EVE_CoDl_end(s_pHalContext);
    EVE_CoDl_restoreContext(s_pHalContext);
	Display_End(s_pHalContext);

    SAMAPP_DELAY;
#endif
}

void SAMAPP_Bitmap() 
{
    SAMAPP_Bitmap_getImage();
    SAMAPP_Bitmap_nonSquareDisplay();
    SAMAPP_Bitmap_dithering();
    SAMAPP_Bitmap_ASTCLayoutRAMG();
    SAMAPP_Bitmap_ASTCLayoutFlash();
    SAMAPP_Bitmap_ASTCMultiCellRAMG();
    SAMAPP_Bitmap_ASTCMultiCellFlash(); 
    SAMAPP_Bitmap_ASTCLayoutCell_1_3x2();
    SAMAPP_Bitmap_rotate();
    SAMAPP_Bitmap_rotateAndTranslate();
    SAMAPP_Bitmap_loadImageMono();
    SAMAPP_Bitmap_loadImageFullColor();
    SAMAPP_Bitmap_DXT1();
	SAMAPP_Bitmap_DXT1L2();
	SAMAPP_Bitmap_DXT1PALETTED();
	SAMAPP_Bitmap_DXT1L2PALETTED();
    SAMAPP_Bitmap_paletted8();
    SAMAPP_Bitmap_paletted4444();
    SAMAPP_Bitmap_higherResolutionBitmap();
}


