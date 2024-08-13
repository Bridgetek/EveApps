/**
 * @file DemoGauges.c
 * @brief Gauges demo
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
#include "DemoGauges.h"

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void DemoGauges();

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
	{ "Gauges demo",
		"Support QVGA, WQVGA, WVGA",
		"EVE1/2/3/4",
		"WIN32, FT9XX, IDM2040"
	};

	while (TRUE) {
		WelcomeScreen(s_pHalContext, info);
		DemoGauges();
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
// FIXME: #include "assets.h"
static uint8_t NUM_DISPLAY_SCREEN;
static int32_t ox;
static int32_t px;

static int VertextFmt = 4; 
static int VertextDiv = 1; 

static void cs(uint8_t i) {
	switch (i) {
	case  0: EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(200, 255, 200)); break;
	case 60: EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 255, 0)); break;
	case 80: EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 0, 0)); break;
	}
}

void Math_Polar(EVE_HalContext *pHalContext, int32_t r, float_t th, int32_t ox, int32_t oy){
    int32_t x, y;
    Math_Polarxy(r, th, &x, &y, ox, oy);
    EVE_Cmd_wr32(s_pHalContext,VERTEX2F(x/VertextDiv, y/VertextDiv));
}

void DemoGauges() {
	uint16_t w, h, n, a, size_d, temp[1024];
	float_t th;
	int32_t rval, tgt;
	uint16_t dloffset;
	uint8_t i, bi, z;
	int32_t x, y, tx, ty, o, dt;
	int16_t xvalue, yvalue;
#ifdef FT801_ENABLE
	int32_t val = 0;
#else
	int16_t val = 0;
#endif

	dt = 10;
	w = s_pHalContext->Width - dt * 4;
	h = s_pHalContext->Height - 20;
	NUM_DISPLAY_SCREEN = 2;

	if (s_pHalContext->Width <= 320)  { // 320 = size width of QVGA
		NUM_DISPLAY_SCREEN = 1;
	}

	if (s_pHalContext->Width >= 1023) { // -2048 to 2047
		VertextFmt = 3;
		VertextDiv = 2;
	}

	EVE_CoCmd_dlStart(s_pHalContext);
	EVE_CoCmd_memSet(s_pHalContext, 0, 0, 10 * 1024);
	EVE_Cmd_waitFlush(s_pHalContext);

	// FIXME: EVE_Cmd_wr32(s_pHalContext, CMD_INFLATE);
	// FIXME: EVE_Cmd_wr32(s_pHalContext, 0);
#if defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM)
	// FIXME: EVE_Cmd_wrMem(s_pHalContext, digits, sizeof(digits));
#else
	// FIXME: EVE_Cmd_wrProgMem(s_pHalContext, digits, sizeof(digits));
#endif

	EVE_CoCmd_dlStart(s_pHalContext);
	EVE_CoCmd_setFont(s_pHalContext, 13, 0);
	EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(13));
	EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(144 - (32L * (54 / 2) * 87)));
	EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(L4, 54 / 2, 87));
	EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, 54, 87));
	EVE_Cmd_wr32(s_pHalContext, DISPLAY());
	EVE_CoCmd_swap(s_pHalContext);
	EVE_Cmd_waitFlush(s_pHalContext);

	EVE_CoCmd_dlStart(s_pHalContext);// dl = 0;
	EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_RGB(55, 55, 55));
	EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
	EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_RGB(0, 0, 0));

	y = 10;
	for (z = 0; z < NUM_DISPLAY_SCREEN; z++) {
		ox = (s_pHalContext->Width / 2)*z;
		px = (s_pHalContext->Width / (2 * NUM_DISPLAY_SCREEN)) + (z*(s_pHalContext->Width / (NUM_DISPLAY_SCREEN)));
#ifdef DISPLAY_RESOLUTION_HVGA_PORTRAIT
		ox = 300 * z;
#endif

		EVE_Cmd_wr32(s_pHalContext, SCISSOR_XY(ox + dt, y));

#ifdef DISPLAY_RESOLUTION_HVGA_PORTRAIT
		EVE_Cmd_wr32(s_pHalContext, SCISSOR_SIZE(w, h));
#else
		EVE_Cmd_wr32(s_pHalContext, SCISSOR_SIZE(w, h + 30));
#endif
		EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(LINES));
		EVE_Cmd_wr32(s_pHalContext, LINE_WIDTH(10));
		for (bi = 0; bi < 81; bi += 10) {
			cs(bi);
			for (i = 2; i < 10; i += 2) {
				a = Math_Da(bi + i, 45);
				Math_Polar(s_pHalContext, 220, a, px, 300);
				Math_Polar(s_pHalContext, 240, a, px, 300);
			}
		}

		EVE_Cmd_wr32(s_pHalContext, LINE_WIDTH(16));
		for (i = 0; i < 91; i += 10) {
			cs(i);
			a = Math_Da(i, 45);
			Math_Polar(s_pHalContext, 220, a, px, 300);
			Math_Polar(s_pHalContext, 250, a, px, 300);
		}
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 255, 255));
		for (i = 0; i < 91; i += 10) {
			a = Math_Da(i, 45);
			Math_Polarxy(260, a, &tx, &ty, px, 300);
			EVE_CoCmd_number(s_pHalContext, tx >> 4, ty >> 4, 26, OPT_CENTER, i);
		}

		if (z == 1)EVE_CoCmd_text(s_pHalContext, px, h - 10, 28, OPT_CENTERX, "Random");
#if defined FT801_ENABLE
		if (z == 0)EVE_CoCmd_text(s_pHalContext, px, h - 10, 28, OPT_CENTERX, "X Position");
#else
		if (z == 0)EVE_CoCmd_text(s_pHalContext, px, h - 10, 28, OPT_CENTERX, "Resistance");
#endif
	}
	EVE_Cmd_waitFlush(s_pHalContext);

	dloffset = EVE_Hal_rd16(s_pHalContext, REG_CMD_DL);

	EVE_Cmd_wr32(s_pHalContext, CMD_MEMCPY);
	EVE_Cmd_wr32(s_pHalContext, 100000L);
	EVE_Cmd_wr32(s_pHalContext, RAM_DL);
	EVE_Cmd_wr32(s_pHalContext, dloffset);
	y = 10 + 120 + 20;
	rval = 0, tgt = 4500;
	int rval2 = 9000L;
	do {
		EVE_CoCmd_dlStart(s_pHalContext);
		EVE_Cmd_wr32(s_pHalContext, VERTEX_FORMAT(VertextFormat));

		EVE_CoCmd_append(s_pHalContext, 100000L, dloffset);

		for (z = 0; z < NUM_DISPLAY_SCREEN; z++) {
			ox = (s_pHalContext->Width / 2)*z;
			px = (s_pHalContext->Width / (2 * NUM_DISPLAY_SCREEN)) + (z*(s_pHalContext->Width / (NUM_DISPLAY_SCREEN)));
			if (z == 0) {
				int reg_touch_rz = EVE_Hal_rd32(s_pHalContext, REG_TOUCH_RZ);
#ifdef FT801_ENABLE
				val = EVE_Hal_rd32(s_pHalContext, REG_CTOUCH_TOUCH0_XY);

				xvalue = ((val >> 16));

				if (xvalue != -32768) {
					val = (xvalue * 899 / s_pHalContext->Width);
				}
				else
					val = 0;
#else
				val = 10 * MIN(899, reg_touch_rz);
#endif
#ifdef BT8XXEMU_PLATFORM
				if (reg_touch_rz != 0x7FFF) {
						int d = random(200) - 100;
						rval2 += d;
						rval2 = rval2>9000L?9000L: rval2;
						rval2 = rval2<7000L?7000L: rval2;
					val = rval2;
				}
				else {
					val = 9000L;
				}
#endif
			}
			else  if (z == 1) {
				int d = (tgt - rval) / 16;
				rval += d;
				if (random(60) == 0)
					tgt = random(9000L);
				val = rval;
			}
			EVE_Cmd_wr32(s_pHalContext, SCISSOR_XY(ox + dt, 10));
			EVE_Cmd_wr32(s_pHalContext, SCISSOR_SIZE(w, 120));
			EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 255, 255));
			EVE_Cmd_wr32(s_pHalContext, BEGIN(LINES));
			EVE_Cmd_wr32(s_pHalContext, LINE_WIDTH(10));
			th = ((uint16_t)val - 4500L) * 32768 / 36000L;
			for (o = -5; o < 6; o++) {
				Math_Polar(s_pHalContext, 170, th + (o << 5), px, 300);
				Math_Polar(s_pHalContext, 235, th, px, 300);
			}
			EVE_Cmd_wr32(s_pHalContext, SCISSOR_XY(ox + dt, y));
			EVE_Cmd_wr32(s_pHalContext, SCISSOR_SIZE(w, (uint16_t)(s_pHalContext->Height*0.36)));
			EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
			EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 0, 0));

#if defined(DISPLAY_RESOLUTION_HVGA_PORTRAIT)
			// Use custom font
			int offset = 105;
			EVE_CoCmd_number(s_pHalContext, px  - offset     , 160, 13, 2, val / 100);
			EVE_CoCmd_text(s_pHalContext  , px  - offset + 86, 160, 13, 0, ".");
			EVE_CoCmd_number(s_pHalContext, px  - offset + 96, 160, 13, 2, val % 100);
#else				
			// Use built in font
			int offset = 70;
			EVE_CoCmd_number(s_pHalContext, px  - offset + 15, 160, 31, 2, val / 100);
			EVE_CoCmd_text(s_pHalContext,   px  - offset + 66, 160, 31, 0, ".");
			EVE_CoCmd_number(s_pHalContext, px  - offset + 76, 160, 31, 2, val % 100); 
#endif
		}
		EVE_Cmd_wr32(s_pHalContext, DISPLAY());
		EVE_CoCmd_swap(s_pHalContext);
		EVE_Cmd_waitFlush(s_pHalContext);
	} while (1);
}

