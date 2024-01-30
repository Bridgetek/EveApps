/**
 * @file DemoSketch.c
 * @brief Sketch demo
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
#include "DemoSketch.h"

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void DemoSketch();

int main(int argc, char* argv[])
{
	s_pHalContext = &s_halContext;
	Gpu_Init(s_pHalContext);

	// read and store calibration setting
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
	Esd_Calibrate(s_pHalContext);
	Calibration_Save(s_pHalContext);
#endif

	Flash_Init(s_pHalContext, TEST_DIR "/Flash/BT81X_Flash.bin", "BT81X_Flash.bin");
	EVE_Util_clearScreen(s_pHalContext);

	char* info[] =
	{ "Washing machine UI demo",
		"Support QVGA, WQVGA, WVGA",
		"EVE1/2/3/4",
		"WIN32, FT9XX, IDM2040"
	};

	while (TRUE) {
		WelcomeScreen(s_pHalContext, info);
		DemoSketch();
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

/* Application*/
void DemoSketch() {
	uint32_t  tracker, color = 0;
	uint16_t  val = 32768;
	uint8_t tag = 0;
	//  Set the bitmap properties , sketch properties and Tracker for the sliders
	EVE_CoCmd_dlStart(s_pHalContext);
	EVE_CoCmd_fgColor(s_pHalContext, 0xffffff);        // Set the bg color
	EVE_CoCmd_track(s_pHalContext, (s_pHalContext->Width - 30), 40, 8, s_pHalContext->Height - 100, 1);

#if defined FT801_ENABLE
	EVE_CoCmd_cSketch(s_pHalContext, 0, 10, s_pHalContext->Width - 40, s_pHalContext->Height - 30, 0, L8, 1500L);
#elif defined FT81X_ENABLE
	EVE_CoCmd_sketch(s_pHalContext, 0, 10, s_pHalContext->Width - 40, s_pHalContext->Height - 30, 0, L8);
#else
	EVE_CoCmd_sketch(s_pHalContext, 0, 10, s_pHalContext->Width - 40, s_pHalContext->Height - 30, 0, L8);
#endif
	EVE_CoCmd_memZero(s_pHalContext, 0L, (s_pHalContext->Width - 40) * (s_pHalContext->Height - 20L));
	EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(1));
	EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(0));
	EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(L8, s_pHalContext->Width - 40, s_pHalContext->Height - 20));
#ifdef FT81X_ENABLE
	EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT_H((s_pHalContext->Width - 40) >> 10, (s_pHalContext->Height - 20) >> 9));
#endif
	EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, (s_pHalContext->Width - 40), (s_pHalContext->Height - 20)));
#ifdef FT81X_ENABLE
	EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H((s_pHalContext->Width - 40) >> 9, (s_pHalContext->Height - 20) >> 9));
#endif
	EVE_CoCmd_swap(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);
	while (1)
	{
		// Check the tracker
		tracker = EVE_Hal_rd32(s_pHalContext, REG_TRACKER);
		// Check the Tag 
		tag = EVE_Hal_rd8(s_pHalContext, REG_TOUCH_TAG);
		//  clear the GRAM when user enter the Clear button
		if (tag == 2)
		{
			EVE_CoCmd_dlStart(s_pHalContext);
			EVE_CoCmd_memZero(s_pHalContext, 0, (s_pHalContext->Width - 40) * (s_pHalContext->Height - 20L)); // Clear the gram frm 1024         
			EVE_Cmd_waitFlush(s_pHalContext);
		}
		// compute the color from the tracker
		if ((tracker & 0xff) == 1)      // check the tag val
		{
			val = (tracker >> 16);
		}
		color = val * 255L;
		// Start the new display list
		EVE_CoCmd_dlStart(s_pHalContext);             // Start the display list
		EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));  // clear the display     
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 255, 255));  // color    
		EVE_CoCmd_bgColor(s_pHalContext, color);
		EVE_Cmd_wr32(s_pHalContext, TAG_MASK(1));
		EVE_Cmd_wr32(s_pHalContext, TAG(1));          // assign the tag value 
		EVE_CoCmd_fgColor(s_pHalContext, color);
		// draw the sliders 
		EVE_CoCmd_slider(s_pHalContext, (s_pHalContext->Width - 30), 40, 8, (s_pHalContext->Height - 100), 0, val, 65535);     // slide j1 cmd  
		EVE_CoCmd_fgColor(s_pHalContext, (tag == 2) ? 0x0000ff : color);
		EVE_Cmd_wr32(s_pHalContext, TAG(2));          // assign the tag value 
		EVE_CoCmd_button(s_pHalContext, (s_pHalContext->Width - 35), (s_pHalContext->Height - 45), 35, 25, 26, 0, "CLR");
		EVE_Cmd_wr32(s_pHalContext, TAG_MASK(0));

		EVE_CoCmd_text(s_pHalContext, s_pHalContext->Width - 35, 10, 26, 0, "Color");

		EVE_Cmd_wr32(s_pHalContext, LINE_WIDTH(1 * 16));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(RECTS));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(0, 10 * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F((int16_t)(s_pHalContext->Width - 40) * 16, (int16_t)(s_pHalContext->Height - 20) * 16));

		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB((color >> 16) & 0xff, (color >> 8) & 0xff, (color) & 0xff));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(0, 10, 1, 0));
		EVE_Cmd_wr32(s_pHalContext, END());
		EVE_Cmd_wr32(s_pHalContext, DISPLAY());
		EVE_CoCmd_swap(s_pHalContext);
		EVE_Cmd_waitFlush(s_pHalContext);
	}
}
