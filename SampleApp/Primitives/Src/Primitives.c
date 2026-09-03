/**
 * @file Primitives.c
 * @brief Sample usage of primitives drawing
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
#include "Primitives.h"

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void SAMAPP_Primitives();

/* Header of raw data containing properties of bitmap */
SAMAPP_Bitmap_header_t SAMAPP_Bitmap_RawData_Header[] =
{
    /* format,width,height,stride,arrayoffset */
    { RGB565      ,    40,      40,    40 * 2,    0 },
};

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
        "This sample demonstrate the using of primitives graphics", 
        "",
        ""
    }; 

    while (TRUE) {
        WelcomeScreen(s_pHalContext, info);

        SAMAPP_Primitives();

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
* @brief linear interpolation
*
* @param t deltaTime
* @param a transform position
* @param b target position
* @return float
*/
float helperLerp(float t, float a, float b)
{
    return (1 - t) * a + t * b;
}

/**
* @brief Smooth linear interpolation
*
* @param t deltaTime
* @param a transform position
* @param b target position
* @return float
*/
static float helperSmoothLerp(float t, float a, float b)
{
    float lt = 3 * t * t - 2 * t * t * t;
    return helperLerp(lt, a, b);
}

/**
* @brief display few points at various offsets with various colors
*
*/
void SAMAPP_Primitives_points()
{
    Draw_Text(s_pHalContext, "Example for: Points");

    /* Construct DL of points */
	Display_StartColor(s_pHalContext, (uint8_t[]) { 128, 128, 128 }, (uint8_t[]) { 128, 0, 0 });
	EVE_CoDl_begin(s_pHalContext, POINTS);
	EVE_CoDl_pointSize(s_pHalContext, 5 * 16);
    EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 5), VP(s_pHalContext->Height / 2));
	EVE_CoDl_colorRgb(s_pHalContext, 0, 128, 0);
	EVE_CoDl_pointSize(s_pHalContext, 15 * 16);
    EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 2 / 5), VP(s_pHalContext->Height / 2));
	EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 128);
	EVE_CoDl_pointSize(s_pHalContext, 25 * 16);
    EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 3 / 5), VP(s_pHalContext->Height / 2));
	EVE_CoDl_colorRgb(s_pHalContext, 128, 128, 0);
	EVE_CoDl_pointSize(s_pHalContext, 35 * 16);
    EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 4 / 5), VP(s_pHalContext->Height / 2));
	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief Display Lines on screen
*
*/
void SAMAPP_Primitives_lines()
{
    Draw_Text(s_pHalContext, "Example for: Lines");

    int16_t LineHeight = 25;

    Display_Start(s_pHalContext);
	EVE_CoDl_begin(s_pHalContext, LINES);
	EVE_CoDl_colorRgb(s_pHalContext, 128, 0, 0);
    EVE_CoDl_lineWidth(s_pHalContext, 5 * 16);
    EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 4), VP((s_pHalContext->Height - LineHeight) / 2));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 4), VP((s_pHalContext->Height + LineHeight) / 2));
	EVE_CoDl_colorRgb(s_pHalContext, 0, 128, 0);
	EVE_CoDl_lineWidth(s_pHalContext, 10 * 16);
    LineHeight = 40;
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 2 / 4), VP((s_pHalContext->Height - LineHeight) / 2));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 2 / 4), VP((s_pHalContext->Height + LineHeight) / 2));
	EVE_CoDl_colorRgb(s_pHalContext, 128, 128, 0);
	EVE_CoDl_lineWidth(s_pHalContext, 20 * 16);
    LineHeight = 55;
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 3 / 4), VP((s_pHalContext->Height - LineHeight) / 2));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 3 / 4), VP((s_pHalContext->Height + LineHeight) / 2));
	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief Display rectangles on screen
*
*/
void SAMAPP_Primitives_rectangles()
{
    Draw_Text(s_pHalContext, "Example for: Rectangles");

    int16_t RectWidth;
    int16_t RectHeight;

    Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 0, 0, 128 });
    EVE_CoDl_begin(s_pHalContext, RECTS);
	EVE_CoDl_lineWidth(s_pHalContext, 1 * 16); //LINE_WIDTH is used for corner curvature
    RectWidth = 5;
    RectHeight = 25;
    EVE_CoDl_vertex2f(s_pHalContext, VP((s_pHalContext->Width / 4) - (RectWidth / 2)), VP((s_pHalContext->Height - RectHeight) / 2));
	EVE_CoDl_vertex2f(s_pHalContext, VP((s_pHalContext->Width / 4) + (RectWidth / 2)), VP((s_pHalContext->Height + RectHeight) / 2));
	EVE_CoDl_colorRgb(s_pHalContext, 0, 128, 0);
	EVE_CoDl_lineWidth(s_pHalContext, 5 * 16);
    RectWidth = 10;
    RectHeight = 40;
	EVE_CoDl_vertex2f(s_pHalContext, VP((s_pHalContext->Width * 2 / 4) - (RectWidth / 2)), VP((s_pHalContext->Height - RectHeight) / 2));
	EVE_CoDl_vertex2f(s_pHalContext, VP((s_pHalContext->Width * 2 / 4) + (RectWidth / 2)), VP((s_pHalContext->Height + RectHeight) / 2));
	EVE_CoDl_colorRgb(s_pHalContext, 128, 128, 0);
	EVE_CoDl_lineWidth(s_pHalContext, 10 * 16);
    RectWidth = 20;
    RectHeight = 55;
	EVE_CoDl_vertex2f(s_pHalContext, VP((s_pHalContext->Width * 3 / 4) - (RectWidth / 2)), VP((s_pHalContext->Height - RectHeight) / 2));
	EVE_CoDl_vertex2f(s_pHalContext, VP((s_pHalContext->Width * 3 / 4) + (RectWidth / 2)), VP((s_pHalContext->Height + RectHeight) / 2));
	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}
/**
* @brief Display image at different locations with various colors, -ve offsets, alpha blend
*
*/
void SAMAPP_Primitives_bitmap()
{
    Draw_Text(s_pHalContext, "Example for: Display image");

    const SAMAPP_Bitmap_header_t* p_bmhdr;
    uint32_t BMoffsetx;
    uint32_t BMoffsety;

    p_bmhdr = &SAMAPP_Bitmap_RawData_Header[0];
    /* Copy raw data into address 0 followed by generation of bitmap */
    Ftf_Write_File_nBytes_To_RAM_G(s_pHalContext, TEST_DIR "\\SAMAPP_Bitmap_RawData.bin", RAM_G,
        p_bmhdr->Stride * p_bmhdr->Height, p_bmhdr->Arrayoffset);

    Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 255, 255, 255 });
	EVE_CoDl_begin(s_pHalContext, BITMAPS); // start drawing bitmaps
#if defined(FT81X_ENABLE)
	EVE_CoCmd_setBitmap(s_pHalContext, RAM_G, p_bmhdr->Format, p_bmhdr->Width, p_bmhdr->Height);
#else
	EVE_CoDl_bitmapSource(s_pHalContext, RAM_G);
	EVE_CoDl_bitmapLayout(s_pHalContext, p_bmhdr->Format, p_bmhdr->Stride, p_bmhdr->Height);
	EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, p_bmhdr->Width, p_bmhdr->Height);
#endif
    BMoffsetx = ((s_pHalContext->Width / 4) - (p_bmhdr->Width / 2));
    BMoffsety = ((s_pHalContext->Height / 2) - (p_bmhdr->Height / 2));
    EVE_CoDl_vertex2f(s_pHalContext, VP(BMoffsetx), VP(BMoffsety));
	EVE_CoDl_colorRgb(s_pHalContext, 255, 64, 64); // red
    BMoffsetx = ((s_pHalContext->Width * 2 / 4) - (p_bmhdr->Width / 2));
    BMoffsety = ((s_pHalContext->Height / 2) - (p_bmhdr->Height / 2));
	EVE_CoDl_vertex2f(s_pHalContext, VP(BMoffsetx), VP(BMoffsety));
	EVE_CoDl_colorRgb(s_pHalContext, 64, 180, 64); // green
    BMoffsetx += (p_bmhdr->Width / 2);
    BMoffsety += (p_bmhdr->Height / 2);
    EVE_CoDl_vertex2f(s_pHalContext, VP(BMoffsetx), VP(BMoffsety));
	EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 64); // transparent yellow
	EVE_CoDl_colorA(s_pHalContext, 150);
    BMoffsetx += (p_bmhdr->Width / 2);
    BMoffsety += (p_bmhdr->Height / 2);
    EVE_CoDl_vertex2f(s_pHalContext, VP(BMoffsetx), VP(BMoffsety));
	EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
	EVE_CoDl_colorA(s_pHalContext, 255);

    EVE_CoDl_vertex2f(s_pHalContext, VP(-10), VP(-10)); //for -ve coordinates use vertex2f instruction
	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief Display a Bar Graph
*
*/
void SAMAPP_Primitives_barGraph()
{
#if EVE_SUPPORT_GEN > EVE1
#define SAMAPP_BARGRAPH_ARRAY_SIZE (256)
    /* Write the data into RAM_G */
    uint8_t Y_Array[SAMAPP_BARGRAPH_ARRAY_SIZE];
    uint32_t numchunks = 0;
    int32_t String_size;
    uint32_t hoffset = 0;
    uint32_t voffset = 0;
    uint32_t widthaligh;

    Draw_Text(s_pHalContext, "Example for: Display a Bar Graph");

    hoffset = 0;
    voffset = (s_pHalContext->Height - 256) / 2; //centre of the screen

    widthaligh = ALIGN_TWO_POWER_N(s_pHalContext->Width, SAMAPP_BARGRAPH_ARRAY_SIZE);
    numchunks = widthaligh / SAMAPP_BARGRAPH_ARRAY_SIZE;
    String_size = SAMAPP_BARGRAPH_ARRAY_SIZE;
    for (uint32_t j = 0; j < numchunks; j++)
    {
        for (int i = 0; i < SAMAPP_BARGRAPH_ARRAY_SIZE; i++)
        {
            Y_Array[i] = random(128) + 64; //within range
        }
        EVE_Hal_wrMem(s_pHalContext, RAM_G + j * SAMAPP_BARGRAPH_ARRAY_SIZE, &Y_Array[0],
            String_size);
    }

	Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 128, 0, 0 });
	EVE_CoDl_scissorXY(s_pHalContext, 0, 0);
	EVE_CoDl_scissorSize(s_pHalContext, 1024, 1024);
	EVE_CoDl_begin(s_pHalContext, BITMAPS); // start drawing bitmaps
	EVE_CoDl_bitmapSource(s_pHalContext, RAM_G);
	EVE_CoDl_bitmapLayout(s_pHalContext, BARGRAPH, 256, 1);
	EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, 256, 256);
    /* Display text 8x8 at hoffset, voffset location */
    for (uint32_t i = 0; i < numchunks; i++)
    {
		EVE_CoDl_cell(s_pHalContext, i);
		EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset));
        hoffset += SAMAPP_BARGRAPH_ARRAY_SIZE;
    }
	EVE_CoDl_end(s_pHalContext);
    Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();

    /* drawing of sine wave with rising amplitude */
    String_size = SAMAPP_BARGRAPH_ARRAY_SIZE;
    for (uint32_t j = 0; j < numchunks; j++)
    {
        for (int i = 0; i < SAMAPP_BARGRAPH_ARRAY_SIZE; i++)
        {
            int32_t tmpval;
            int16_t tmpidx;
            tmpidx = (int16_t) (i + j * SAMAPP_BARGRAPH_ARRAY_SIZE);
            tmpval = 128 + ((tmpidx / 4) * Math_Qsin(65536 * tmpidx / 48) / 65536); //within range

            Y_Array[i] = tmpval & 0xff;
        }
        EVE_Hal_wrMem(s_pHalContext, RAM_G + j * SAMAPP_BARGRAPH_ARRAY_SIZE, Y_Array, String_size);
    }

    SAMAPP_ENABLE_DELAY();

	Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 255, 255, 255 });
	EVE_CoDl_scissorXY(s_pHalContext, 0, 0);
	EVE_CoDl_scissorSize(s_pHalContext, 1024, 1024);
	EVE_CoDl_begin(s_pHalContext, BITMAPS); // start drawing bitmaps
	EVE_CoDl_bitmapSource(s_pHalContext, RAM_G);
	EVE_CoDl_bitmapLayout(s_pHalContext, BARGRAPH, 256, 1);
	EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, 256, 256);

    /* Display bargraph at hoffset, voffset location */
    voffset = (s_pHalContext->Height - 256) / 2; //centre of the screen
    hoffset = 0;
    for (uint32_t i = 0; i < numchunks; i++)
    {
        EVE_CoDl_colorRgb(s_pHalContext, 0xff, 0, 0);
		EVE_CoDl_cell(s_pHalContext, i);
		EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset));

        EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
        EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset + 4));
        hoffset += SAMAPP_BARGRAPH_ARRAY_SIZE;
    }
	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();

#if EVE_SUPPORT_GEN > EVE2
	EVE_Util_loadRawFile(s_pHalContext, RAM_G, TEST_DIR "\\bargraph_1000x256_8000_bytes.raw");
	Display_Start(s_pHalContext);
	EVE_CoDl_colorRgb(s_pHalContext, 29, 222, 58);
	EVE_CoCmd_setBitmap(s_pHalContext, RAM_G, BARGRAPH, 1000, 256);
	EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_vertex2f(s_pHalContext, 0, 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(-16), 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(16), 0);

	EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
	EVE_CoDl_vertex2f(s_pHalContext, 0, VP(16));
	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
	SAMAPP_ENABLE_DELAY();
#endif
	EVE_CoDl_scissorSize(s_pHalContext, 2048, 2048);
#endif
}

/**
* @brief Display Linestrips
*
*/
void SAMAPP_Primitives_lineStrips()
{
    Draw_Text(s_pHalContext, "Example for: Linestrips");

	Display_StartColor(s_pHalContext, (uint8_t[]) { 5, 45, 10 }, (uint8_t[]) { 255, 168, 64 });
    EVE_CoDl_begin(s_pHalContext, LINE_STRIP);
    EVE_CoDl_vertex2f(s_pHalContext, VP(16), VP(16));
    EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 2 / 3), VP(s_pHalContext->Height * 2 / 3));
    EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width - 80), VP(s_pHalContext->Height - 20));
	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief Display Edgestrips
*
*/
void SAMAPP_Primitives_edgeStrips()
{
    Draw_Text(s_pHalContext, "Example for: Edgestrips");

	Display_StartColor(s_pHalContext, (uint8_t[]) { 5, 45, 10 }, (uint8_t[]) { 255, 168, 64 });
	EVE_CoDl_begin(s_pHalContext, EDGE_STRIP_R);
	EVE_CoDl_vertex2f(s_pHalContext, VP(16), VP(16));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 2 / 3), VP(s_pHalContext->Height * 2 / 3));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width - 80), VP(s_pHalContext->Height - 20));
	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief Scissor sample
*
*/
void SAMAPP_Primitives_scissor()
{
    Draw_Text(s_pHalContext, "Example for: Scissor");

	EVE_CoDl_saveContext(s_pHalContext);
	Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 255, 255, 255 });
	EVE_CoDl_scissorXY(s_pHalContext, 40, 20); // Scissor rectangle top left
	EVE_CoDl_scissorSize(s_pHalContext, 40, 40); // Scissor rectangle is 400 x 400 pixels
	EVE_CoDl_clearColorRgb(s_pHalContext, 255, 255, 0); // Clear to yellow
	EVE_CoDl_clear(s_pHalContext, 1, 1, 1);
	Display_End(s_pHalContext);
	EVE_CoDl_restoreContext(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief Stencil sample
*
*/
void SAMAPP_Primitives_stencil()
{
    Draw_Text(s_pHalContext, "Example for: Stencil");

    int16_t PointSize = s_pHalContext->Height / 7;
    int16_t DispBtwPoints = 60;

	Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 0, 0, 128 });
	EVE_CoDl_stencilOp(s_pHalContext, INCR, INCR);
	EVE_CoDl_pointSize(s_pHalContext, PointSize * 16);
	EVE_CoDl_begin(s_pHalContext, POINTS);
	EVE_CoDl_vertex2f(s_pHalContext, VP((s_pHalContext->Width - DispBtwPoints) / 2), VP(s_pHalContext->Height / 2));
	EVE_CoDl_vertex2f(s_pHalContext, VP((s_pHalContext->Width + DispBtwPoints) / 2), VP(s_pHalContext->Height / 2));
	EVE_CoDl_stencilFunc(s_pHalContext, EQUAL, 2, 255);
	EVE_CoDl_colorRgb(s_pHalContext, 128, 0, 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2), VP(s_pHalContext->Height / 2));
	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief Display Polygons
*
*/
void SAMAPP_Primitives_polygon()
{
    Draw_Text(s_pHalContext, "Example for: Polygons");

    Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 255, 0, 0 });
	EVE_CoDl_stencilOp(s_pHalContext, INCR, INCR);
	EVE_CoDl_colorMask(s_pHalContext, 0, 0, 0, 0); //mask all the colors
	EVE_CoDl_begin(s_pHalContext, EDGE_STRIP_L);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2), VP(s_pHalContext->Height / 4));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 4 / 5), VP(s_pHalContext->Height * 4 / 5));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 4), VP(s_pHalContext->Height / 2));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2), VP(s_pHalContext->Height / 4));
	EVE_CoDl_end(s_pHalContext);
	EVE_CoDl_colorMask(s_pHalContext, 1, 1, 1, 1); //enable all the colors
	EVE_CoDl_stencilFunc(s_pHalContext, EQUAL, 1, 255);
	EVE_CoDl_begin(s_pHalContext, EDGE_STRIP_L);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width), 0);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width), VP(s_pHalContext->Height));
	EVE_CoDl_end(s_pHalContext);

	/* Draw lines at the borders to make sure anti aliazing is also done */
	EVE_CoDl_stencilFunc(s_pHalContext, ALWAYS, 0, 255);
	EVE_CoDl_lineWidth(s_pHalContext, 1 * 16);
	EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 0);
	EVE_CoDl_begin(s_pHalContext, LINES);
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2), VP(s_pHalContext->Height / 4));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 4 / 5), VP(s_pHalContext->Height * 4 / 5));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width * 4 / 5), VP(s_pHalContext->Height * 4 / 5));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 4), VP(s_pHalContext->Height / 2));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 4), VP(s_pHalContext->Height / 2));
	EVE_CoDl_vertex2f(s_pHalContext, VP(s_pHalContext->Width / 2), VP(s_pHalContext->Height / 4));
	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief Display a cube
*
*/
void SAMAPP_Primitives_cube()
{
	Draw_Text(s_pHalContext, "Example for: Cube");
    uint32_t points[6 * 5][2], x, y, i, z;
    int16_t xoffset, yoffset, CubeEdgeSz;

    // color vertices
	uint8_t colors[6][3] = { 255, 0, 0,
		255, 128, 0,
		255, 255, 0,
		0, 255, 0,
		0, 0, 255,
		128, 0, 255 };

    // Cube dimention is of 100*100*100
    CubeEdgeSz = 100;
    xoffset = (s_pHalContext->Width / 2 - CubeEdgeSz); 
    yoffset = ((s_pHalContext->Height - CubeEdgeSz) / 2);
	x = (xoffset + (CubeEdgeSz / 2)); //     xoff+w/2
	y = (yoffset - (CubeEdgeSz / 2)); //     yoff-h/2

    //yz plane (back)
	points[0][0] = x;
	points[0][1] = y;
	points[1][0] = x;
	points[1][1] = y + CubeEdgeSz;
	points[2][0] = x + CubeEdgeSz;
	points[2][1] = y + CubeEdgeSz;
	points[3][0] = x + CubeEdgeSz;
	points[3][1] = y;
	points[4][0] = x;
	points[4][1] = y;

	//yz plane (left)
	points[5][0] = xoffset;
	points[5][1] = yoffset;
	points[6][0] = x;
	points[6][1] = y;
	points[7][0] = x;
	points[7][1] = y + CubeEdgeSz;
	points[8][0] = xoffset;
	points[8][1] = yoffset + CubeEdgeSz;
	points[9][0] = xoffset;
	points[9][1] = yoffset;

	//xz plane(top)
	points[10][0] = xoffset;
	points[10][1] = yoffset;
	points[11][0] = xoffset + CubeEdgeSz;
	points[11][1] = yoffset;
	points[12][0] = x + CubeEdgeSz;
	points[12][1] = y;
	points[13][0] = x;
	points[13][1] = y;
	points[14][0] = xoffset;
	points[14][1] = yoffset;

	//xz plane(bottom)
	points[15][0] = xoffset;
	points[15][1] = yoffset + CubeEdgeSz;
	points[16][0] = xoffset + CubeEdgeSz;
	points[16][1] = yoffset + CubeEdgeSz;
	points[17][0] = x + CubeEdgeSz;
	points[17][1] = y + CubeEdgeSz;
	points[18][0] = x;
	points[18][1] = y + CubeEdgeSz;
	points[19][0] = xoffset;
	points[19][1] = yoffset + CubeEdgeSz;

	//yz plane (right)
	points[20][0] = xoffset + CubeEdgeSz;
	points[20][1] = yoffset + CubeEdgeSz;
	points[21][0] = x + CubeEdgeSz;
	points[21][1] = y + CubeEdgeSz;
	points[22][0] = x + CubeEdgeSz;
	points[22][1] = y;
	points[23][0] = xoffset + CubeEdgeSz;
	points[23][1] = yoffset;
	points[24][0] = xoffset + CubeEdgeSz;
	points[24][1] = yoffset + CubeEdgeSz;

	// xy plane(front)
	points[25][0] = xoffset;
	points[25][1] = yoffset;
	points[26][0] = xoffset + CubeEdgeSz;
	points[26][1] = yoffset;
	points[27][0] = xoffset + CubeEdgeSz;
	points[27][1] = yoffset + CubeEdgeSz;
	points[28][0] = xoffset;
	points[28][1] = yoffset + CubeEdgeSz;
	points[29][0] = xoffset;
	points[29][1] = yoffset;

    Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 255, 255, 255 });

	// Draw a cube
	EVE_CoDl_stencilOp(s_pHalContext, INCR, INCR);
	EVE_CoDl_colorA(s_pHalContext, 192);
    for (z = 0; z < 6; z++)
    {
		EVE_CoDl_clear(s_pHalContext, 0, 1, 1); //clear stencil buffer
		EVE_CoDl_colorMask(s_pHalContext, 0, 0, 0, 0); //mask all the colors and draw one surface
		EVE_CoDl_stencilFunc(s_pHalContext, ALWAYS, 0, 255); //stencil function to increment all the values
		EVE_CoDl_begin(s_pHalContext, EDGE_STRIP_L);
		for (i = 0; i < 5; i++)
		{
			EVE_CoDl_vertex2f(s_pHalContext, VP(points[z * 5 + i][0]), VP(points[z * 5 + i][1]));
		}
		EVE_CoDl_end(s_pHalContext);
		/* set the color and draw a strip */
		EVE_CoDl_colorMask(s_pHalContext, 1, 1, 1, 1);
		EVE_CoDl_stencilFunc(s_pHalContext, EQUAL, 1, 255);
		EVE_CoDl_colorRgb(s_pHalContext, colors[z][0], colors[z][1], colors[z][2]);
		EVE_CoDl_begin(s_pHalContext, RECTS);
		EVE_CoDl_vertex2f(s_pHalContext, VP(xoffset), 0);
		EVE_CoDl_vertex2f(s_pHalContext, VP(xoffset + CubeEdgeSz * 2), VP(yoffset + CubeEdgeSz * 2));
		EVE_CoDl_end(s_pHalContext);
    }
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief draw points followed by lines to create 3d ball kind of effect
*
*/
void SAMAPP_Primitives_ballStencil()
{
    Draw_Text(s_pHalContext, "Example for: Display a ball");

    int32_t xball = (s_pHalContext->Width / 2);
	int32_t yball = (s_pHalContext->Height / 2);
    int32_t rball = 100;
    int32_t numpoints = 6;
    int32_t numlines = 8;
    int32_t asize;
    int32_t aradius;
    int32_t gridsize = 50;
    int32_t asmooth;
    int32_t loopflag = 100;
    int32_t dispr = (s_pHalContext->Width - 10);
    int32_t displ = 10;
    int32_t dispa = 10;
    int32_t dispb = (s_pHalContext->Height - 10);
    int32_t xflag = 1;
    int32_t yflag = 1;

    dispr -= ((dispr - displ) % gridsize);
    dispb -= ((dispb - dispa) % gridsize);
    /* write the play sound */
    EVE_Hal_wr16(s_pHalContext, REG_SOUND, 0x50);
    EVE_Hal_wr8(s_pHalContext, REG_VOL_SOUND, 100);
    while (loopflag-- > 0)
    {
        if (((xball + rball + 2) >= dispr) || ((xball - rball - 2) <= displ))
        {
            xflag ^= 1;
            EVE_Hal_wr8(s_pHalContext, REG_PLAY, 1);
        }
        if (((yball + rball + 8) >= dispb) || ((yball - rball - 8) <= dispa))
        {
            yflag ^= 1;
            EVE_Hal_wr8(s_pHalContext, REG_PLAY, 1);
        }
        if (xflag)
        {
            xball += 2;
        }
        else
        {
            xball -= 2;
        }
        if (yflag)
        {
            yball += 8;
        }
        else
        {
            yball -= 8;
        }

        Display_StartColor(s_pHalContext, (uint8_t[]) { 128, 128, 0 }, (uint8_t[]) { 0, 0, 0 });
        /* draw grid */
        EVE_CoDl_lineWidth(s_pHalContext, 1 * 16);
        EVE_CoDl_begin(s_pHalContext, LINES);
        for (int i = 0; i <= ((dispr - displ) / gridsize); i++)
        {
            EVE_CoDl_vertex2f(s_pHalContext, VP(displ + i * gridsize), VP(dispa));
            EVE_CoDl_vertex2f(s_pHalContext, VP(displ + i * gridsize), VP(dispb));
        }
        for (int i = 0; i <= ((dispb - dispa) / gridsize); i++)
        {
            EVE_CoDl_vertex2f(s_pHalContext, VP(displ), VP(dispa + i * gridsize));
            EVE_CoDl_vertex2f(s_pHalContext, VP(dispr), VP(dispa + i * gridsize));
        }
        EVE_CoDl_end(s_pHalContext);

        EVE_CoDl_colorMask(s_pHalContext, 0, 0, 0, 0); //mask all the colors
		EVE_CoDl_pointSize(s_pHalContext, rball * 16);
		EVE_CoDl_begin(s_pHalContext, POINTS);
		EVE_CoDl_vertex2f(s_pHalContext, VP(xball), VP(yball));
		EVE_CoDl_stencilOp(s_pHalContext, INCR, ZERO);
		EVE_CoDl_stencilFunc(s_pHalContext, GEQUAL, 1, 255);
        /* one side points */

        for (int i = 1; i <= numpoints; i++)
        {
            asize = i * rball * 2 / (numpoints + 1);
            asmooth = (int16_t) helperSmoothLerp(((float) (1.0 * asize / (2 * rball))), 0,
                (float) (2 * rball));

            if (asmooth > rball)
            {
                //change the offset to -ve
                int32_t tempsmooth;
                tempsmooth = asmooth - rball;
                aradius = (rball * rball + tempsmooth * tempsmooth) / (2 * tempsmooth);
				EVE_CoDl_pointSize(s_pHalContext, aradius * 16);
				EVE_CoDl_vertex2f(s_pHalContext, VP(xball - aradius + tempsmooth), VP(yball));
            }
            else
            {
                int32_t tempsmooth;
                tempsmooth = rball - asmooth;
                aradius = (rball * rball + tempsmooth * tempsmooth) / (2 * tempsmooth);
				EVE_CoDl_pointSize(s_pHalContext, aradius * 16);
				EVE_CoDl_vertex2f(s_pHalContext, VP(xball + aradius - tempsmooth), VP(yball));
            }
        }
        EVE_CoDl_end(s_pHalContext);

		EVE_CoDl_begin(s_pHalContext, LINES);
        /* draw lines - line should be at least radius diameter */
        for (int i = 1; i <= numlines; i++)
        {
            asize = (i * rball * 2 / numlines);
            asmooth = (int16_t) helperSmoothLerp(((float) (1.0 * asize / (2 * rball))), 0,
                (float) (2 * rball));
			EVE_CoDl_lineWidth(s_pHalContext, asmooth * 16);
			EVE_CoDl_vertex2f(s_pHalContext, VP(xball - rball), VP(yball - rball));
			EVE_CoDl_vertex2f(s_pHalContext, VP(xball + rball), VP(yball - rball));
        }
		EVE_CoDl_end(s_pHalContext);

        EVE_CoDl_colorMask(s_pHalContext, 1, 1, 1, 1); // enable all the colors
		EVE_CoDl_stencilFunc(s_pHalContext, ALWAYS, 1, 255);
		EVE_CoDl_stencilOp(s_pHalContext, KEEP, KEEP);
		EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
		EVE_CoDl_pointSize(s_pHalContext, rball * 16);
		EVE_CoDl_begin(s_pHalContext, POINTS);

        EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 0); //shadow
		EVE_CoDl_colorA(s_pHalContext, 160);
        EVE_CoDl_vertex2f(s_pHalContext, VP(xball + 16), VP(yball + 16));

        EVE_CoDl_colorRgb(s_pHalContext, 255, 255, 255);
		EVE_CoDl_colorA(s_pHalContext, 255);
		EVE_CoDl_vertex2f(s_pHalContext, VP(xball), VP(yball));

        EVE_CoDl_colorRgb(s_pHalContext, 255, 0, 0);
		EVE_CoDl_stencilFunc(s_pHalContext, GEQUAL, 1, 1);
		EVE_CoDl_stencilOp(s_pHalContext, KEEP, KEEP);
		EVE_CoDl_vertex2f(s_pHalContext, VP(xball), VP(yball));
		EVE_CoDl_end(s_pHalContext);
		Display_End(s_pHalContext);

        EVE_sleep(30);
    }
}

/**
* @brief demonstrated display of point and text
*
*/
void SAMAPP_Primitives_string()
{
    int16_t hoffset;
    int16_t voffset;

    Draw_Text(s_pHalContext, "Example for: Display a text");

    voffset = (int16_t) ((s_pHalContext->Height - 49) / 2); //the max height of inbuilt font handle 31
    hoffset = (int16_t) ((s_pHalContext->Width - 4 * 60) / 2);

    Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 0, 0, 0 }); // clear screen
    EVE_CoDl_begin(s_pHalContext, BITMAPS); // start drawing bitmaps

	EVE_CoDl_bitmapHandle(s_pHalContext, 31);
	EVE_CoDl_cell(s_pHalContext, 'B');
    EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset)); // ascii B in font 31
    hoffset += 24;
	EVE_CoDl_cell(s_pHalContext, 'R');
	EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset)); // ascii R
    hoffset += 26;
	EVE_CoDl_cell(s_pHalContext, 'I');
	EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset)); // ascii I
    hoffset += 10;
	EVE_CoDl_cell(s_pHalContext, 'D');
	EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset)); // ascii D
    hoffset += 24;
	EVE_CoDl_cell(s_pHalContext, 'G');
	EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset)); // ascii G
    hoffset += 24;
	EVE_CoDl_cell(s_pHalContext, 'E');
	EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset)); // ascii E
    hoffset += 24;
	EVE_CoDl_cell(s_pHalContext, 'T');
	EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset)); // ascii T
    hoffset += 24;
	EVE_CoDl_cell(s_pHalContext, 'E');
	EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset)); // ascii E
    hoffset += 24;
	EVE_CoDl_cell(s_pHalContext, 'K');
	EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset)); // ascii K
	EVE_CoDl_end(s_pHalContext);
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief Additive blending of points - 1000 points
*
*/
void SAMAPP_Primitives_additiveBlendPoints()
{
    int32_t hoffset;
    int32_t hoffsetdiff;
    int32_t voffset;
    int32_t flagloop = 1;
    int32_t hdiff;
    int32_t vdiff;
    int32_t PointSz;

    Draw_Text(s_pHalContext, "Example for: Additive blending of points");

#define MSVC_PI (3.141592653)
    PointSz = s_pHalContext->Width / 100;
    flagloop = 10;
    hoffsetdiff = s_pHalContext->Width / 160;
    while (flagloop-- > 0)
    {
        /* Download the DL into DL RAM */
		Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 20, 91, 20 });
		EVE_CoDl_blendFunc(s_pHalContext, SRC_ALPHA, DST_ALPHA); //input is source alpha and destination is whole color
		EVE_CoDl_pointSize(s_pHalContext, PointSz * 16);
		EVE_CoDl_begin(s_pHalContext, POINTS);

        /* First 100 random values */
        for (int i = 0; i < 100; i++)
        {
            hoffset = random(s_pHalContext->Width);
            voffset = random(s_pHalContext->Height);
			EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset));
        }

        /* next 480 are sine values of two cycles */
        for (int i = 0; i < 160; i++)
        {
            /* i is x offset, y is sinwave */
            hoffset = i * hoffsetdiff;

            voffset = (s_pHalContext->Height / 2)
                + ((int32_t) (s_pHalContext->Height / 2)
                    * Math_Qsin((uint16_t) (65536 * i / (s_pHalContext->Width / 6)) / 65536));

            EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset));
            for (int j = 0; j < 4; j++)
            {
                hdiff = random(PointSz * 6) - (PointSz * 3);
                vdiff = random(PointSz * 6) - (PointSz * 3);
				EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset + hdiff), VP(voffset + vdiff));
            }
        }

        EVE_CoDl_end(s_pHalContext);
		Display_End(s_pHalContext);
        EVE_sleep(10);
    }
}

/**
* @brief display text8x8 of abcdefgh
*
*/
void SAMAPP_Primitives_text8x8()
{
    Draw_Text(s_pHalContext, "Example for: Text8x8");

    /* Write the data into RAM_G */
    const uint8_t Text_Array[] = "abcdefgh";
    int32_t String_size;
    int32_t hoffset = 16;
    int32_t voffset = 16;

    String_size = sizeof(Text_Array) - 1;
    EVE_Hal_wrMem(s_pHalContext, RAM_G, Text_Array, String_size);

    /*
    abcdefgh
    abcdefgh
    */

    Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 0, 0, 0 });
    EVE_CoDl_bitmapSource(s_pHalContext, RAM_G);
    EVE_CoDl_bitmapLayout(s_pHalContext, TEXT8X8, 1 * 8, 1); //L1 format, each input data element is in 1 byte size
    EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, REPEAT, 8 * 8, 8 * 2); //output is 8x8 format - draw 8 characters in horizontal repeated in 2 line

    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    /* Display text 8x8 at hoffset, voffset location */
    EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset));

    /*
    abcdabcdabcdabcd
    efghefghefghefgh
    */
	EVE_CoDl_bitmapLayout(s_pHalContext, TEXT8X8, 1 * 4, 2); //L1 format and each datatype is 1 byte size
	EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, REPEAT, BORDER, 8 * 16, 8 * 2); //each character is 8x8 in size -  so draw 32 characters in horizontal and 32 characters in vertical
    hoffset = s_pHalContext->Width / 2;
    voffset = s_pHalContext->Height / 2;
	EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset));
    EVE_CoDl_end(s_pHalContext);

    Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief display textVGA of random values
*
*/
void SAMAPP_Primitives_textVGA()
{
    Draw_Text(s_pHalContext, "Example for: TextVGA");

    /* Write the data into RAM_G */
    uint16_t Text_Array[160];
    int32_t String_size;
    int32_t hoffset = 32;
    int32_t voffset = 32;

    for (int i = 0; i < 160; i++)
    {
        Text_Array[i] = (uint16_t) random(65536); //within range
    }

    String_size = 160 * 2;
    EVE_Hal_wrMem(s_pHalContext, RAM_G, (uint8_t*) Text_Array, String_size);

    Display_Start(s_pHalContext); // clear screen
    EVE_CoDl_bitmapSource(s_pHalContext, RAM_G);

    /* mandatory for textvga as background color is also one of the parameter in textvga format */
    EVE_CoDl_blendFunc(s_pHalContext, ONE, ZERO);

    //draw 8x8
    EVE_CoDl_bitmapLayout(s_pHalContext, TEXTVGA, 2 * 4, 8); //L1 format, but each input data element is of 2 bytes in size
    EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, 8 * 8, 8 * 8); //output is 8x8 format - draw 8 characters in horizontal and 8 vertical
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset));
    EVE_CoDl_end(s_pHalContext);

    //draw textvga
	EVE_CoDl_bitmapLayout(s_pHalContext, TEXTVGA, 2 * 16, 8); //L1 format but each datatype is 16bit size
	EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, REPEAT, 8 * 32, 8 * 32); //8 pixels per character and 32 rows/colomns
	EVE_CoDl_begin(s_pHalContext, BITMAPS);
    hoffset = s_pHalContext->Width / 2;
    voffset = s_pHalContext->Height / 2;
    /* Display textvga at hoffset, voffset location */
	EVE_CoDl_vertex2f(s_pHalContext, VP(hoffset), VP(voffset));
	EVE_CoDl_end(s_pHalContext);

    Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief usage of additive blending - draw 3 Gs
*
*/
void SAMAPP_Primitives_additiveBlendText()
{
    Draw_Text(s_pHalContext, "Example for: Additive blending");

    Display_StartColor(s_pHalContext, (uint8_t[]) { 255, 255, 255 }, (uint8_t[]) { 0, 0, 0 });
	EVE_CoCmd_text(s_pHalContext, 50, 30, 31, 0, "Bridgetek");
	EVE_CoDl_colorA(s_pHalContext, 128);
	EVE_CoCmd_text(s_pHalContext, 58, 38, 31, 0, "Bridgetek");
	EVE_CoDl_colorA(s_pHalContext, 64);
	EVE_CoCmd_text(s_pHalContext, 66, 46, 31, 0, "Bridgetek");
	Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

/**
* @brief Usage of macro
*
*/
void SAMAPP_Primitives_macroUsage()
{
    Draw_Text(s_pHalContext, "Example for: Macro");

    int32_t xoffset;
    int32_t yoffset;
    int32_t xflag = 1;
    int32_t yflag = 1;
    int32_t flagloop = 1;
    const SAMAPP_Bitmap_header_t* p_bmhdr;

    xoffset = s_pHalContext->Width / 3;
    yoffset = s_pHalContext->Height / 2;

    /* First write a valid macro instruction into macro0 */
    EVE_Hal_wr32(s_pHalContext, REG_MACRO_0, VERTEX2F(VP(xoffset), VP(yoffset)));
    /* update lena face as bitmap 0 */

    p_bmhdr = &SAMAPP_Bitmap_RawData_Header[0];
    /* Copy raw data into address 0 followed by generation of bitmap */

    Ftf_Write_File_nBytes_To_RAM_G(s_pHalContext, TEST_DIR "\\SAMAPP_Bitmap_RawData.bin", RAM_G,
        p_bmhdr->Stride * p_bmhdr->Height, p_bmhdr->Arrayoffset);

    Display_Start(s_pHalContext);	
	EVE_CoDl_begin(s_pHalContext, BITMAPS); // start drawing bitmaps
    EVE_CoDl_bitmapSource(s_pHalContext, RAM_G);
    EVE_CoDl_bitmapLayout(s_pHalContext, p_bmhdr->Format, p_bmhdr->Stride, p_bmhdr->Height);
    EVE_CoDl_bitmapSize(s_pHalContext, NEAREST, BORDER, BORDER, p_bmhdr->Width, p_bmhdr->Height);
    EVE_CoDl_macro(s_pHalContext, 0); // draw the image at (100,120)
    EVE_CoDl_end(s_pHalContext);
    Display_End(s_pHalContext);
    flagloop = 300;
    while (flagloop-- > 0)
    {
        if (((uint32_t)(xoffset + p_bmhdr->Width) >= s_pHalContext->Width) || (xoffset <= 0))
        {
            xflag ^= 1;
        }
		if (((uint32_t)(yoffset + p_bmhdr->Height) >= s_pHalContext->Height) || (yoffset <= 0))
        {
            yflag ^= 1;
        }
        if (xflag)
        {
            xoffset++;
        }
        else
        {
            xoffset--;
        }
        if (yflag)
        {
            yoffset++;
        }
        else
        {
            yoffset--;
        }
        /*  update just the macro */
        EVE_Hal_wr32(s_pHalContext, REG_MACRO_0, VERTEX2F(VP(xoffset), VP(yoffset)));
        EVE_sleep(10);
    }
}

/**
* @brief API to demonstrate for CMD_CALIBRATESUB
*
*/
void SAMAPP_Primitives_calibratesub() {
#if EVE_SUPPORT_GEN == EVE4
    Draw_Text(s_pHalContext, "Example for: CMD_CALIBRATESUB");

    uint16_t clb_x = 20;
    uint16_t clb_y = 20;
    uint16_t clb_w = (uint16_t)(s_pHalContext->Width / 2);
    uint16_t clb_h = (uint16_t)(s_pHalContext->Height / 2);

    Display_Start(s_pHalContext);
    EVE_Util_loadImageFile(s_pHalContext, RAM_G, TEST_DIR "\\Mountain_800x480_RGB565_Converted.png", NULL, OPT_RGB565);
    //Start drawing bitmap
    EVE_CoDl_saveContext(s_pHalContext);
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
    EVE_CoDl_vertex2f(s_pHalContext, 0, 0);
    EVE_CoDl_end(s_pHalContext);

    EVE_CoDl_colorRgb(s_pHalContext, 0, 255, 255);
    EVE_CoCmd_text(s_pHalContext, clb_x + 260, clb_y + clb_h / 2, 30, 0,
        "Please touch on screen");

    EVE_CoCmd_fgColor(s_pHalContext, 0xb90007);
    EVE_CoCmd_calibrateSub(s_pHalContext, clb_x, clb_y, clb_w, clb_h);

    EVE_CoDl_restoreContext(s_pHalContext);
    EVE_Cmd_waitFlush(s_pHalContext);
#endif
}

/**
* @brief API to demonstrate CMD_TESTCARD
*
*/
void SAMAPP_Primitives_testcard() {
#if EVE_SUPPORT_GEN == EVE4
    Draw_Text(s_pHalContext, "Example for: CMD_TESTCARD");

    EVE_CoCmd_testCard(s_pHalContext);
    EVE_Cmd_waitFlush(s_pHalContext);
    EVE_sleep(2000);
#endif
}

/**
* @brief API to demonstrate the use of transfer commands
*
*/
void SAMAPP_Primitives_appendComand()
{
    uint32_t AppendCmds[16];
    int16_t xoffset;
    int16_t yoffset;

    Draw_Text(s_pHalContext, "Example for: Appending DL command");

    /*************************************************************************/
    /* Below code demonstrates the usage of coprocessor append command - to append any*/
    /* mcu specific graphics commands to coprocessor generated graphics commands      */
    /*************************************************************************/

    /* Bitmap construction by MCU - display lena at 200x60 offset */
    /* Construct the bitmap data to be copied in the temp buffer then by using
    coprocessor append command copy it into graphics processor DL memory */
    xoffset = (int16_t) ((s_pHalContext->Width - SAMAPP_Bitmap_RawData_Header[0].Width) / 2);
    yoffset = (int16_t) ((s_pHalContext->Height / 3) - SAMAPP_Bitmap_RawData_Header[0].Height / 2);

    EVE_Cmd_wr32(s_pHalContext, CMD_DLSTART);
    AppendCmds[0] = CLEAR_COLOR_RGB(255, 0, 0);
    AppendCmds[1] = CLEAR(1, 1, 1);
    AppendCmds[2] = COLOR_RGB(255, 255, 255);
    AppendCmds[3] = BEGIN(BITMAPS);
    AppendCmds[4] = BITMAP_SOURCE(0);
    AppendCmds[5] = BITMAP_LAYOUT(SAMAPP_Bitmap_RawData_Header[0].Format,
        SAMAPP_Bitmap_RawData_Header[0].Stride, SAMAPP_Bitmap_RawData_Header[0].Height);
    AppendCmds[6] = BITMAP_SIZE(BILINEAR, BORDER, BORDER, SAMAPP_Bitmap_RawData_Header[0].Width,
        SAMAPP_Bitmap_RawData_Header[0].Height);
    AppendCmds[7] = VERTEX2F(VP(xoffset), VP(yoffset));
    AppendCmds[8] = END();

    /* Download the bitmap data*/
    Ftf_Write_File_nBytes_To_RAM_G(s_pHalContext, TEST_DIR "\\SAMAPP_Bitmap_RawData.bin", RAM_G,
        SAMAPP_Bitmap_RawData_Header[0].Stride * SAMAPP_Bitmap_RawData_Header[0].Height,
        SAMAPP_Bitmap_RawData_Header[0].Arrayoffset);

    /* Download the DL data constructed by the MCU to location 40*40*2 in sram */
    EVE_Hal_wrMem(s_pHalContext,
        RAM_G + SAMAPP_Bitmap_RawData_Header[0].Stride * SAMAPP_Bitmap_RawData_Header[0].Height,
        (uint8_t*) AppendCmds, 9 * 4);

    /* Call the append api for copying the above generated data into graphics processor
    DL commands are stored at location 40*40*2 offset from the starting of the sram */
    EVE_CoCmd_append(s_pHalContext,
        RAM_G + SAMAPP_Bitmap_RawData_Header[0].Stride * SAMAPP_Bitmap_RawData_Header[0].Height, 9 * 4);
    /*  Display the text information */
    EVE_CoCmd_fgColor(s_pHalContext, 0xffff00);
    xoffset -= 50;
    yoffset += 40;
    EVE_CoCmd_text(s_pHalContext, xoffset, yoffset, 26, 0, "Display bitmap by Append");
    EVE_Cmd_wr32(s_pHalContext, DISPLAY());
    EVE_CoCmd_swap(s_pHalContext);
    /* Download the commands into fifo */
    EVE_Cmd_waitFlush(s_pHalContext);
    EVE_sleep(2000);
}

/**
* @brief Simple graph
*
*/
void SAMAPP_Primitives_simpleMap()
{
    Draw_Text(s_pHalContext, "Example for: A simple map");
	EVE_CoCmd_dlStart(s_pHalContext);
    EVE_CoDl_clearColorRgb(s_pHalContext, 236, 232, 224); //light gray
    EVE_CoDl_clear(s_pHalContext, 1, 1, 1);
    EVE_CoDl_colorRgb(s_pHalContext, 170, 157, 136); //medium gray
    EVE_CoDl_lineWidth(s_pHalContext, 63);
    EVE_CoDl_call(s_pHalContext, 18); //draw the streets
    EVE_CoDl_colorRgb(s_pHalContext, 250, 250, 250); //white
    EVE_CoDl_lineWidth(s_pHalContext, 48);
    EVE_CoDl_call(s_pHalContext, 18); //draw the streets
    EVE_CoDl_colorRgb(s_pHalContext, 0, 0, 0);
    EVE_CoDl_begin(s_pHalContext, BITMAPS);
	EVE_CoDl_vertex2ii(s_pHalContext, 240, 91, 27, 77); //draw "Main st." at (240,91)
	EVE_CoDl_vertex2ii(s_pHalContext, 252, 91, 27, 97);
	EVE_CoDl_vertex2ii(s_pHalContext, 260, 91, 27, 105);
	EVE_CoDl_vertex2ii(s_pHalContext, 263, 91, 27, 110);
	EVE_CoDl_vertex2ii(s_pHalContext, 275, 91, 27, 115);
	EVE_CoDl_vertex2ii(s_pHalContext, 282, 91, 27, 116);
	EVE_CoDl_vertex2ii(s_pHalContext, 286, 91, 27, 46);
    EVE_CoDl_end(s_pHalContext);
    EVE_CoDl_display(s_pHalContext);
    EVE_CoDl_begin(s_pHalContext, LINES);
    EVE_CoDl_vertex2f(s_pHalContext, -160, -20);
	EVE_CoDl_vertex2f(s_pHalContext, 320, 4160);
	EVE_CoDl_vertex2f(s_pHalContext, 800, -20);
	EVE_CoDl_vertex2f(s_pHalContext, 1280, 4160);
	EVE_CoDl_vertex2f(s_pHalContext, 1920, -20);
	EVE_CoDl_vertex2f(s_pHalContext, 2400, 4160);
	EVE_CoDl_vertex2f(s_pHalContext, 2560, -20);
	EVE_CoDl_vertex2f(s_pHalContext, 3040, 4160);
	EVE_CoDl_vertex2f(s_pHalContext, 3200, -20);
	EVE_CoDl_vertex2f(s_pHalContext, 3680, 4160);
	EVE_CoDl_vertex2f(s_pHalContext, 2880, -20);
	EVE_CoDl_vertex2f(s_pHalContext, 3360, 4160);
	EVE_CoDl_vertex2f(s_pHalContext, -20, 0);
	EVE_CoDl_vertex2f(s_pHalContext, 5440, -480);
	EVE_CoDl_vertex2f(s_pHalContext, -20, 960);
	EVE_CoDl_vertex2f(s_pHalContext, 5440, 480);
	EVE_CoDl_vertex2f(s_pHalContext, -20, 1920);
	EVE_CoDl_vertex2f(s_pHalContext, 5440, 1440);
	EVE_CoDl_vertex2f(s_pHalContext, -20, 2880);
	EVE_CoDl_vertex2f(s_pHalContext, 5440, 2400);
	EVE_CoDl_end(s_pHalContext);
    EVE_CoDl_return(s_pHalContext);

    Display_End(s_pHalContext);
    SAMAPP_ENABLE_DELAY();
}

void SAMAPP_Primitives() 
{
    SAMAPP_Primitives_points();
    SAMAPP_Primitives_lines();
    SAMAPP_Primitives_rectangles();
    SAMAPP_Primitives_bitmap();
    SAMAPP_Primitives_barGraph();
    SAMAPP_Primitives_lineStrips();
    SAMAPP_Primitives_edgeStrips();
    SAMAPP_Primitives_scissor();
    SAMAPP_Primitives_stencil();
    SAMAPP_Primitives_polygon();
    SAMAPP_Primitives_cube();
    SAMAPP_Primitives_ballStencil();
    SAMAPP_Primitives_string();
    SAMAPP_Primitives_additiveBlendPoints();
    SAMAPP_Primitives_text8x8();
    SAMAPP_Primitives_textVGA();
    SAMAPP_Primitives_additiveBlendText();
    SAMAPP_Primitives_macroUsage();
    SAMAPP_Primitives_calibratesub();
    SAMAPP_Primitives_testcard();
    SAMAPP_Primitives_appendComand();
    SAMAPP_Primitives_simpleMap();
}


