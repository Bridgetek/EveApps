/**
 * @file DemoSignals.c
 * @brief Wave signals demo
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
#include "DemoSignals.h"

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void DemoSignals();

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
		"EVE1/2/3/4",
		"WIN32, FT9XX, IDM2040"
	};

	while (TRUE) {
		WelcomeScreen(s_pHalContext, info);
		DemoSignals();
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
static uint8_t rate = 2;
static int16_t x, y, tx;
static uint8_t beats[10];
static char8_t beats_Incr[10] = { -10,10,5,-5,-20,20,12,-12,-5,5 };
static uint16_t add2write = 0;

static bool_t istouch()
{
	return !(EVE_Hal_rd16(s_pHalContext, REG_TOUCH_RAW_XY) & 0x8000);
}

void Sine_wave(uint8_t amp)
{
	static uint8_t played = 0, change = 0;
	x += rate;
	if (x > s_pHalContext->Width) x = 0;
	y = (s_pHalContext->Height / 2) + ((int32_t)amp * Math_Qsin(-65536 * x / (25 * rate)) / 65536);
	if (played == 0 && change < y) {
		played = 1;
		Play_Sound(s_pHalContext, (108 << 8 | 0x10), 50, 100);
	}
	if (change > y)
		played = 0;
	change = y;
	EVE_Hal_wr16(s_pHalContext, RAM_G + (x / rate) * 4, VERTEX2F(x * 16, y * 16));
}

void Sawtooth_wave(uint8_t amp)
{
	static uint16_t temp = 0;
	static uint8_t pk = 0;
	x += rate;
	if (x > s_pHalContext->Width) { x = 0; }
	temp += 2; if (temp > 65535L) temp = 0;
	y = (temp % amp);
	pk = y / (amp - 2);
	if (pk) Play_Sound(s_pHalContext, (108 << 8 | 0x10), 50, 100);
	y = (s_pHalContext->Height / 2) - y;
	EVE_Hal_wr16(s_pHalContext, RAM_G + (x / rate) * 4, VERTEX2F(x * 16, y * 16));
}

void Triangle_wave(uint8_t amp)
{
	static uint16_t temp = 0;
	static uint8_t pk = 0, dc = 0, p = 0;
	x += rate;
	if (x > s_pHalContext->Width) { x = 0; }
	temp += 2; if (temp > 65535L) temp = 0;
	y = (temp % amp);
	pk = (y / (amp - 2)) % 2;
	dc = (temp / amp) % 2;
	if (pk) { if (p == 0) { p = 1; Play_Sound(s_pHalContext, (108 << 8 | 0x10), 50, 100); } else  p = 0; }
	if (dc) y = (s_pHalContext->Height / 2) - (amp - y);  else
		y = (s_pHalContext->Height / 2) - y;
	EVE_Hal_wr16(s_pHalContext, RAM_G + (x / rate) * 4, VERTEX2F(x * 16, y * 16));
}
static uint16_t temp_x, temp_p, temp_y, en;

void Heartbeat()
{
	uint16_t tval;
	y = s_pHalContext->Height / 2;
	for (tval = 0; tval < 10; tval++)
	{
		y = y + (beats_Incr[tval] * 5);
		beats[tval] = y;
	}
	x += rate; if (x > s_pHalContext->Width) {
		x = 0; temp_p = 0; temp_y = 0;
		y = s_pHalContext->Height / 2;
		en = 0; temp_x = 0;
	}
	tx = 5 * rate;
	tx = ((temp_p + 1) * tx) + temp_p * temp_x;
	if (tx <= x) { if (0 == en)	  en = 1; }
	if (en == 1) {
		if (y != beats[temp_y])
		{
			y += beats_Incr[temp_y] * 5;
			temp_y++;
			if (y == (s_pHalContext->Height / 2) + beats_Incr[4] * 5)
				Play_Sound(s_pHalContext, (108 << 8 | 0x10), 50, 100);
		}
		else
		{
			temp_y++;
			if (temp_y > 9) {
				temp_y = 0;   temp_p++;
				en = 0;  temp_x = x - tx;

			}
		}
	}
	EVE_Hal_wr32(s_pHalContext, RAM_G + (x / rate) * 4, VERTEX2F(x * 16, y * 16));
}

void Gpu_Radiobutton(int16_t x, int16_t y, uint32_t bgcolor, uint32_t fgcolor, uint8_t psize, uint8_t tag, uint8_t option)
{
	uint8_t check_size = psize / 2;
	EVE_Cmd_wr32(s_pHalContext, SAVE_CONTEXT());
	EVE_Cmd_wr32(s_pHalContext, COLOR_RGB((bgcolor >> 16 & 0xff), (bgcolor >> 8 & 0xff), (bgcolor & 0xff)));
	EVE_Cmd_wr32(s_pHalContext, TAG(tag));
	EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(psize * 16));
	EVE_Cmd_wr32(s_pHalContext, BEGIN(FTPOINTS));
	EVE_Cmd_wr32(s_pHalContext, VERTEX2F(x * 16, y * 16));
	if (tag == option)
	{
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB((fgcolor >> 16 & 0xff), (fgcolor >> 8 & 0xff), (fgcolor & 0xff)));
		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(check_size * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(x * 16, y * 16));
	}
	EVE_Cmd_wr32(s_pHalContext, RESTORE_CONTEXT());
}

void DemoSignals() {
	int16_t i, xx = 0;
	uint16_t tval, th_to, amp;
	uint8_t  temp[480], hide_x = 0, fg = 1, tag, opt = 3;
	// ========  Bg =========================================================
	for (tval = 0; tval <= s_pHalContext->Height / 2; tval++)
	{
		temp[s_pHalContext->Height - tval] = temp[tval] = (tval * 0.90);
	}
	EVE_Hal_wrMem(s_pHalContext, s_pHalContext->Width * 4L, temp, sizeof(temp));

	y = s_pHalContext->Height / 2;
	for (tval = 0; tval < 10; tval++)
	{
		y = y + (beats_Incr[tval] * 5);
		beats[tval] = y;
	}

	for (tval = 0; tval < s_pHalContext->Width; tval += rate)
	{
		EVE_Hal_wr32(s_pHalContext, RAM_G + ((tval / rate) * 4), VERTEX2F(tval * 16, y * 16));
	}
	add2write = 0;

	while (1)
	{
		// ========  Menu  =========================================================
		if (istouch())   fg = 1;
		if (fg) {
			th_to = 0; if (hide_x > 0)hide_x = 0; else
				fg = 0;
		}
		else
		{
			th_to++;
			if (th_to > 250) {
				if (hide_x < 150) hide_x++;   else
					th_to = 0;
			}
		}
		//==========Option =========================================================    
		tag = Gesture_GetTag(s_pHalContext);
		if (tag != 0)
		{
			x = 0;   temp_p = 0; en = 0; temp_x = 0; temp_y = 0; //reset
			if (tag > 2)  opt = tag;
			if (tag == 1)if (rate > 1)rate--;
			if (tag == 2)if (rate < 6)rate++;
			y = (s_pHalContext->Height / 2);
			for (tval = 0; tval < s_pHalContext->Width; tval += rate)
			{
				EVE_Hal_wr32(s_pHalContext, RAM_G + ((tval / rate) * 4), VERTEX2F(tval * 16, y * 16));
			}
			add2write = 0;
		}
		//========= Signals ========================================================
		amp = 100;
		switch (opt)
		{
		case 5:

			Triangle_wave(amp);
			break;

		case 4:
			Sawtooth_wave(amp);
			break;

		case 3:
			Sine_wave(amp);
			break;

		case 6:
			amp = 50;
			Heartbeat();
			break;
		}
		//=========Display list start===================================================    
		EVE_CoCmd_dlStart(s_pHalContext);
		EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0x12, 0x4A, 0x26));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(s_pHalContext->Width * 4L));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(L8, 1, s_pHalContext->Height));
#ifdef DISPLAY_RESOLUTION_WVGA
		EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT_H(0, s_pHalContext->Height >> 9));
#endif
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, REPEAT, BORDER, s_pHalContext->Width, s_pHalContext->Height));
#ifdef DISPLAY_RESOLUTION_WVGA
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H(s_pHalContext->Width >> 9, s_pHalContext->Height >> 9));
#endif
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
		EVE_Cmd_wr32(s_pHalContext, TAG(0));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(0, 0));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0x1B, 0xE0, 0x67));

		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0x1B, 0xE0, 0x67));
		EVE_Cmd_wr32(s_pHalContext, LINE_WIDTH(2 * 16));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(LINE_STRIP));
		EVE_CoCmd_append(s_pHalContext, RAM_G, (x / rate) * 4);
		EVE_Cmd_wr32(s_pHalContext, END());

		EVE_Cmd_wr32(s_pHalContext, BEGIN(LINE_STRIP));
		if ((x / rate) < (s_pHalContext->Width / rate) - (50 / rate))            // else it screw up
			EVE_CoCmd_append(s_pHalContext, RAM_G + (x / rate) * 4 + ((50 / rate) * 4), ((s_pHalContext->Width / rate) * 4) - ((x / rate) * 4) - ((50 / rate) * 4));

		EVE_Cmd_wr32(s_pHalContext, END());

		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(6 * 16));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(FTPOINTS));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(x * 16, y * 16));
		EVE_Cmd_wr32(s_pHalContext, END());
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0xff, 0xff, 0xff));
		EVE_Cmd_wr32(s_pHalContext, COLOR_A(100));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(EDGE_STRIP_R));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F((hide_x + s_pHalContext->Width - 100) * 16, 0));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F((hide_x + s_pHalContext->Width - 100) * 16, s_pHalContext->Height * 16));
		EVE_Cmd_wr32(s_pHalContext, COLOR_A(255));
		Gpu_Radiobutton(hide_x + s_pHalContext->Width - 82, s_pHalContext->Height - 90, 0xffffff, 0, 14, 3, opt);
		Gpu_Radiobutton(hide_x + s_pHalContext->Width - 82, s_pHalContext->Height - 55, 0xffffff, 0, 14, 4, opt);
		Gpu_Radiobutton(hide_x + s_pHalContext->Width - 82, s_pHalContext->Height - 20, 0xffffff, 0, 14, 5, opt);
		Gpu_Radiobutton(hide_x + s_pHalContext->Width - 82, s_pHalContext->Height - 125, 0xffffff, 0, 14, 6, opt);
		EVE_CoCmd_text(s_pHalContext, hide_x + s_pHalContext->Width - 65, s_pHalContext->Height - 90, 26, OPT_CENTERY, "Sine");
		EVE_CoCmd_text(s_pHalContext, hide_x + s_pHalContext->Width - 65, s_pHalContext->Height - 55, 26, OPT_CENTERY, "Sawtooth");
		EVE_CoCmd_text(s_pHalContext, hide_x + s_pHalContext->Width - 65, s_pHalContext->Height - 20, 26, OPT_CENTERY, "Triangle");
		EVE_CoCmd_text(s_pHalContext, hide_x + s_pHalContext->Width - 65, s_pHalContext->Height - 125, 26, OPT_CENTERY, "ECG");
		EVE_CoCmd_text(s_pHalContext, (hide_x + s_pHalContext->Width - 70), 20, 30, OPT_CENTERY | OPT_CENTERX, "-");
		EVE_CoCmd_text(s_pHalContext, (hide_x + s_pHalContext->Width - 25), 20, 30, OPT_CENTERY | OPT_CENTERX, "+");
		EVE_CoCmd_text(s_pHalContext, (hide_x + s_pHalContext->Width - 85), 50, 28, 0, "Rate:");
		EVE_CoCmd_number(s_pHalContext, (hide_x + s_pHalContext->Width - 35), 50, 28, 0, rate);
		EVE_CoCmd_text(s_pHalContext, (hide_x + s_pHalContext->Width - 85), 80, 28, 0, "Pk:");
		EVE_CoCmd_number(s_pHalContext, (hide_x + s_pHalContext->Width - 45), 80, 28, 0, amp);
		EVE_Cmd_wr32(s_pHalContext, COLOR_A(50));
		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(15 * 16));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(FTPOINTS));
		EVE_Cmd_wr32(s_pHalContext, TAG(1));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F((hide_x + s_pHalContext->Width - 70) * 16, 20 * 16));
		EVE_Cmd_wr32(s_pHalContext, TAG(2));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F((hide_x + s_pHalContext->Width - 25) * 16, 20 * 16));

		EVE_Cmd_wr32(s_pHalContext, DISPLAY());
		EVE_CoCmd_swap(s_pHalContext);
		EVE_Cmd_waitFlush(s_pHalContext);
		//==========================End==================================================    
	}
}