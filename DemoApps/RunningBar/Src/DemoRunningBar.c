/**
 * @file DemoRunningBar.c
 * @brief Street light moving demo 
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
#include "DemoRunningBar.h"
// Require eve >= FT810 to support PALETTED8
#if EVE_CHIPID >= EVE_FT810

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void DemoRunningBar();

// ************************************ main loop ************************************
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

	char* info[] =
	{ "Wave signals demo",
		"Support QVGA, WQVGA, WVGA",
		"EVE3/4",
		"WIN32, FT9XX, IDM2040"
	};

	while (TRUE) {
		WelcomeScreen(s_pHalContext, info);
		DemoRunningBar();
		EVE_Util_clearScreen(s_pHalContext);
		EVE_Hal_close(s_pHalContext);
		EVE_Hal_release();

		/* Init HW Hal for next loop*/
		Gpu_Init(s_pHalContext);
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
		Calibration_Restore(s_pHalContext);
#endif
	}
	return 0;
}

// ************************************ application ************************************

#define INDICATOR_FIELD_HANDLE    1
#define GLOWIMAGE_HANDLE          2
#define INDICATOR_BAR_HANDLE      3
#define INDICATOR_FRAME_HANDLE    4

#define PALETTED_LUT              (2048)

static uint32_t paletted8_source = RAM_G;

void drawIndicator() {
	static uint8_t  veh_phase = 0;
	/*
	* Draw border of the running bar
	*/
	{
		// set blue color for border bitmap
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0x06, 0x32, 0x47));

		// draw bitmap
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(20, 32, INDICATOR_FRAME_HANDLE, 0));
	}

	/*
	* Draw transparent scissor clip
	*/
	{
		// backup current graphics context
		EVE_Cmd_wr32(s_pHalContext, SAVE_CONTEXT());

		// only draw alpha part of the raw image - to make scissor clip transparent
		EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(0, 0, 0, 1));

		// pixel_color_out = source * 1 + destination * 0: set scissor clip, clear background
		EVE_Cmd_wr32(s_pHalContext, BLEND_FUNC(ONE, ZERO));

		// draw bitmap
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(15, 29, INDICATOR_BAR_HANDLE, 0));
		EVE_Cmd_wr32(s_pHalContext, RESTORE_CONTEXT());
	}
	/*
	* draw running green column on the scissor clip
	*/
	{
		// set green color for running column
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0x76, 0xff, 0x35));

		// pixel_color_out = source * DST_ALPHA + destination * (1 - DST_ALPHA): 
		// ->if destination alpha ==  1: draw green column, else draw dest
		EVE_Cmd_wr32(s_pHalContext, BLEND_FUNC(DST_ALPHA, ONE_MINUS_DST_ALPHA));

		// Move PALETTE_SOURCE offset: to make the column move
		EVE_Cmd_wr32(s_pHalContext, PALETTE_SOURCE(paletted8_source + 4 * veh_phase));

		// draw the bitmap
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(20, 28, INDICATOR_FIELD_HANDLE, 0));
	}

	// increase offset of running green column
	veh_phase += 1; // veh_phase  is 8-bit, if veh_phase > 2^8, veh_phase = 0
}

void setupBitmapHandles() {
	EVE_Cmd_wr32(s_pHalContext, CMD_DLSTART);
	uint32_t ram_image_addr = RAM_G;

	{
		/*Indicator field*/
		uint16_t iw = 296;
		uint16_t ih = 1;
		uint16_t format = L8;
		Gpu_Hal_LoadImageToMemory(s_pHalContext, TEST_DIR "\\indicator_field_296x1_L8.raw", ram_image_addr, LOAD);
		EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(INDICATOR_FIELD_HANDLE));
		EVE_CoCmd_setBitmap(s_pHalContext, ram_image_addr, format, iw, ih);
		ram_image_addr += iw * ih;

		/*paletted8*/
		paletted8_source = ram_image_addr;

		Gpu_Hal_LoadImageToMemory(s_pHalContext, TEST_DIR "\\indicator_field_NewPaletted8.raw", ram_image_addr, LOAD);
		EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(PALETTED8, iw, ih));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT_H(0, 0));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, REPEAT, iw, 57));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H(0, 0));
		ram_image_addr += PALETTED_LUT;
	}

	{
		uint16_t iw = 310;
		uint16_t ih = 57;
		Gpu_Hal_LoadImageToMemory(s_pHalContext, TEST_DIR "\\indicator_bar_310x57_L4.raw", ram_image_addr, LOAD);
		EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(INDICATOR_BAR_HANDLE));
		EVE_CoCmd_setBitmap(s_pHalContext, ram_image_addr, L4, iw, ih);

		ram_image_addr += (iw * ih) / 2;
	}

	{
		uint16_t iw = 304;
		uint16_t ih = 53;
		Gpu_Hal_LoadImageToMemory(s_pHalContext, TEST_DIR "\\indicator_frame_304x53_L4.raw", ram_image_addr, LOAD);
		EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(INDICATOR_FRAME_HANDLE));
		EVE_CoCmd_setBitmap(s_pHalContext, ram_image_addr, L4, iw, ih);
		ram_image_addr += (iw * ih) / 2;
	}
	EVE_Cmd_wr32(s_pHalContext, CMD_SWAP);
	EVE_Cmd_waitFlush(s_pHalContext);
}

void DemoRunningBar() {	
	// init bitmaps
	setupBitmapHandles();

	while (1) {
		Display_Start(s_pHalContext);
		drawIndicator();
		Display_End(s_pHalContext);
	}
}
#else
#warning Platform is not supported
int main(int argc, char* argv[]) {}
#endif