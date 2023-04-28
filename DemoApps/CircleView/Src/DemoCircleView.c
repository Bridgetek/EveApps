/**
 * @file DemoCircleView.c
 * @brief Image viewer demo
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

#include "Platform.h"
#include "Common.h"
#include "App.h"
#if defined(EVE_FLASH_AVAILABLE)

#define RAW  4096
#define LUT  421184 
#define SIZ  (LUT+1024)

static uint16_t isTouch, touchX, touchY;
static EVE_HalContext *s_pHalContext;

void getMousePosition() {
	isTouch = EVE_Hal_rd32(s_pHalContext, REG_TOUCH_RAW_XY) != 0xFFFFFFFF;

	uint32_t x = EVE_Hal_rd16(s_pHalContext, REG_TOUCH_SCREEN_XY + 2);
	uint32_t y = EVE_Hal_rd16(s_pHalContext, REG_TOUCH_SCREEN_XY + 4);

	if (x > s_pHalContext->Width || y > s_pHalContext->Height) {
		isTouch = 0;
	}
	else {
		touchX = x;
		touchY = y;
	}
}

void mousePoint() {
	EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(128, 128, 0));
	EVE_Cmd_wr32(s_pHalContext, BEGIN(POINTS));
	EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(10 * 16));
	EVE_Cmd_wr32(s_pHalContext, VERTEX2F(touchX * 16, touchY * 16));
}
void draw() {
	uint32_t x, y;

	x = touchX * 16;
	y = touchY * 16;

	Display_Start(s_pHalContext);

	EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_RGB(255, 255, 255));
	EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_A(0));
	EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));

	EVE_CoCmd_setBitmap(s_pHalContext, RAW, PALETTED8, 802, 520);

	EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(2600));
	EVE_Cmd_wr32(s_pHalContext, BEGIN(POINTS));
	EVE_Cmd_wr32(s_pHalContext, VERTEX_FORMAT(4));
	EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(0, 0, 0, 1));
	EVE_Cmd_wr32(s_pHalContext, COLOR_A(255));
	EVE_Cmd_wr32(s_pHalContext, VERTEX2F(x, y));

	EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
	//Alpha channel                        
	EVE_Cmd_wr32(s_pHalContext, BLEND_FUNC(DST_ALPHA, ZERO));
	EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(0, 0, 0, 1));
	EVE_Cmd_wr32(s_pHalContext, PALETTE_SOURCE(LUT + 3));
	EVE_Cmd_wr32(s_pHalContext, VERTEX2II(0, 0, 0, 0));
	
	//Red Channel                          
	EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(1, 0, 0, 0));
	EVE_Cmd_wr32(s_pHalContext, PALETTE_SOURCE(LUT + 2));
	EVE_Cmd_wr32(s_pHalContext, VERTEX2II(0, 0, 0, 0));

	// Green Channel                        
	EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(0, 1, 0, 0));
	EVE_Cmd_wr32(s_pHalContext, PALETTE_SOURCE(LUT+1));
	EVE_Cmd_wr32(s_pHalContext, VERTEX2II(0, 0, 0, 0));

	//Blue Channel                         
	EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(0, 0, 1, 0));
	EVE_Cmd_wr32(s_pHalContext, PALETTE_SOURCE(LUT));
	EVE_Cmd_wr32(s_pHalContext, VERTEX2II(0, 0, 0, 0));

	mousePoint();

	Display_End(s_pHalContext);
}

void DemoCircleView(EVE_HalContext* pHalContext) {
	s_pHalContext = pHalContext;
	FlashHelper_SwitchFullMode(s_pHalContext);
	EVE_CoCmd_flashRead(s_pHalContext, 0, 0, SIZ);

	touchX = s_pHalContext->Width / 2;
	touchY = s_pHalContext->Height / 2;

	while (true) {
		draw();
		getMousePosition();
	}	
}
#endif

