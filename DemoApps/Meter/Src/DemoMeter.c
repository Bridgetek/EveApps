/**
 * @file DemoMeter.c
 * @brief Meter demo
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
#include "DemoMeter.h"

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void DemoMeter();

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

	Flash_Init(s_pHalContext, TEST_DIR "/Flash/BT81X_Flash.bin", "BT81X_Flash.bin");
	EVE_Util_clearScreen(s_pHalContext);

	char* info[] =
	{ "Meter demo",
		"Support QVGA, WQVGA, WVGA",
		"EVE1/2/3/4",
		"WIN32, FT900, IDM2040 \n\n This demo has 2 mode: Absolute and relative dial"
	};

	while (TRUE) {
		WelcomeScreen(s_pHalContext, info);
		DemoMeter();
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
#define SAMAPP_Abs_MeterDial_Size146x146_rgb (19005 )
#define SAMAPP_Abs_BaseDial_Size146x146_L1 (1308)
#define SAMAPP_Relative_MeterDial_Size146x146_rgb (18064)

//#define Absolute_Dial
#define Relative_Dial

#define STARTUP_ADDRESS	100*1024L
#define ICON_ADDRESS	250*1024L
#define BLK_LEN	  1024

#define CLOCK_HANDLE	(0x01)

#define DIAL		1

int32_t BaseTrackVal = 0, BaseTrackValInit = 0, BaseTrackValSign = 0, MemLoc = 0;
int16_t MoveDeg = 0;
float_t angle;
/* Global used for Hal Context */
#include "assets.h"

#ifdef Relative_Dial
static struct {
	int32_t dragprev;
	int32_t vel;      // velocity
	int32_t base;    // screen x coordinate, in 1/16ths pixel
}scroller;

static void scroller_init(uint32_t limit)
{
	scroller.dragprev = 0;
	scroller.vel = 0;      // velocity
	scroller.base = 0;     // screen x coordinate, in 1/16ths pixel
}

static void scroller_run()
{
	int32_t change;
	uint32_t sx;
	int32_t sxtemp = 0, sxprev = 0, dragdeg = 0;

	EVE_CoCmd_track(s_pHalContext, (240) + 40, 136, DIAL, DIAL, 1);

	sx = EVE_Hal_rd32(s_pHalContext, REG_TRACKER);
	if ((sx != 0) && (scroller.dragprev != 0))
	{
		scroller.vel = (scroller.dragprev - sx) << 4;
	}
	else
	{
		change = MAX(1, abs(scroller.vel) >> 5);
		if (scroller.vel < 0)
			scroller.vel += change;
		if (scroller.vel > 0)
			scroller.vel -= change;
	}
	scroller.dragprev = sx;
	scroller.base += scroller.vel;
	scroller.base = MAX(0, (scroller.base));
}
#endif

void Load_RawDataFromfile(char ImageArrayname[],/* Image Array*/

	uint32_t ptr,/* Array Size*/
	int16_t Handle,
	int16_t Format,
	int16_t Stride,
	int16_t Width,
	int16_t Height
)
{
	if (Handle == 0)
	{
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(0));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(0));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(L4, 146 / 2, 146));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(BILINEAR, BORDER, BORDER, 146, 146));
		EVE_Cmd_wr32(s_pHalContext, CMD_INFLATE);
		EVE_Cmd_wr32(s_pHalContext, 0/*RamAddr*/);
		EVE_Cmd_wrMem(s_pHalContext, SAMAPP_Abs_Base_Dial_L1, SAMAPP_Abs_BaseDial_Size146x146_L1);
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 255, 255));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(0, 0, 0, 0));
	}
	else if (Handle == 1)
	{
		EVE_Cmd_wr32(s_pHalContext, CMD_INFLATE);
		EVE_Cmd_wr32(s_pHalContext, 15000/*RamAddr*/);

		EVE_Cmd_wrMem(s_pHalContext, ImageArrayname, ptr);
		EVE_Cmd_wr32(s_pHalContext, BITMAP_HANDLE(Handle));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(15000));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(Format, Stride, Height));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(NEAREST, BORDER, BORDER, Width, Height));
#ifdef DISPLAY_RESOLUTION_WVGA
		EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT_H(Stride >> 10, Height >> 9));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H(Width >> 9, Height >> 9));
#endif
	}
}

#ifdef Absolute_Dial
void Abs_Meter_Dial()
{
	uint8_t i = 0, Volume = 0, Read_tag, prevtag = 0;
	int16_t ScreenWidthMid, ScreenHeightMid, th, theta, Touch, NoTouch, Sx1 = 0, Sy1 = 0;
	int16_t OuterCircleRadius, PrevVal = 0, CircleX, CircleY, ButY, ButWid, NoY, Tx1, Tx2, Tx3, Ty1, Ty2, Ty3;

	int32_t PrevTh, CurrTh, adjusting = 0, BaseTrackValSign = 0, PrevBaseVal = 0, Delta = 0, X2, Y2, X1, Y1;

	uint32_t RotaryTag = 0, G1, G2;

	ScreenWidthMid = s_pHalContext->Width / 2;
	ScreenHeightMid = s_pHalContext->Height / 2;

	NoTouch = -32768;

	MoveDeg = 31;

	Tx1 = Tx2 = Tx3 = Ty1 = Ty2 = Ty3 = 0;

#if defined(DISPLAY_RESOLUTION_WQVGA) 
	Sx1 = Sy1 = s_pHalContext->Width;
	OuterCircleRadius = ScreenWidthMid - 144;
	CircleX = ScreenWidthMid;
	CircleY = ScreenHeightMid;
	ButY = s_pHalContext->Height - 262;
	ButWid = 27;
	NoY = (s_pHalContext->Height - 262 + 30 / 2);
	Tx1 = ScreenWidthMid - 86;
	Tx2 = ScreenWidthMid - 70 + 146 / 2;
	Ty2 = ScreenHeightMid - 67 + 146 / 2;
	Tx3 = ScreenWidthMid + 72;

#elif defined(DISPLAY_RESOLUTION_QVGA)
	Sx1 = Sy1 = s_pHalContext->Width;
	ButY = 1;
	ButWid = 23;
	NoY = 12;
	OuterCircleRadius = ScreenWidthMid - 64;
	CircleX = ScreenWidthMid - 70 + 146 / 2;
	CircleY = ScreenHeightMid - 67 + 146 / 2;
	Tx1 = ScreenWidthMid - 63;
	Tx2 = ScreenWidthMid - 70 + 146 / 2;
	Ty2 = ScreenHeightMid - 67 + 146 / 2;
	Tx3 = ScreenWidthMid + 72;
#elif defined(DISPLAY_RESOLUTION_HVGA_PORTRAIT)
	Sx1 = Sy1 = s_pHalContext->Width;
	ButY = 1;
	ButWid = 27;
	NoY = 12;
	OuterCircleRadius = ScreenWidthMid - 64;
	CircleX = ScreenWidthMid - 70 + 146 / 2;
	CircleY = ScreenHeightMid - 67 + 146 / 2;
	Tx1 = ScreenWidthMid - 130;
	Tx2 = ScreenWidthMid - 70 + 146 / 2;
	Ty2 = ScreenHeightMid - 67 + 146 / 2;
	Tx3 = ScreenWidthMid + 140;
#elif defined(DISPLAY_RESOLUTION_WVGA)
	Sx1 = Sy1 = s_pHalContext->Width;
	OuterCircleRadius = ScreenWidthMid - 250;
	CircleX = ScreenWidthMid;
	CircleY = ScreenHeightMid;
	ButY = s_pHalContext->Height - 462;
	ButWid = 45;
	NoY = (s_pHalContext->Height - 462 + 42 / 2);
	Tx1 = ScreenWidthMid - 138;
	Tx2 = ScreenWidthMid - 130 + 253 / 2; //ScreenWidthMid-130 + 243/2;
	Ty2 = ScreenHeightMid - 117 + 243 / 2;
	Tx3 = ScreenWidthMid + 152; //ScreenWidthMid+148;

#else
	Sx1 = Sy1 = s_pHalContext->Width;
	OuterCircleRadius = ScreenWidthMid - 144;
	CircleX = ScreenWidthMid;
	CircleY = ScreenHeightMid;
	ButY = s_pHalContext->Height - 262;
	ButWid = 27;
	NoY = (s_pHalContext->Height - 262 + 30 / 2);
	Tx1 = ScreenWidthMid - 86;
	Tx2 = ScreenWidthMid - 70 + 146 / 2;
	Ty2 = ScreenHeightMid - 67 + 146 / 2;
	Tx3 = ScreenWidthMid + 72;

#endif

	EVE_Cmd_wr32(s_pHalContext, CMD_MEMSET);
	EVE_Cmd_wr32(s_pHalContext, 0L);//starting address of memset
	EVE_Cmd_wr32(s_pHalContext, 55L);//value of memset
	EVE_Cmd_wr32(s_pHalContext, 256 * 1024);

	/* set the vol to MAX */
	EVE_Hal_wr8(s_pHalContext, REG_VOL_SOUND, 0);
	/* Play carousel*/
	EVE_Hal_wr16(s_pHalContext, REG_SOUND, 8);

	/* Construction of starting screen shot, assign all the bitmap handles here */
	EVE_Cmd_wr32(s_pHalContext, CMD_DLSTART);
	EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_RGB(255, 255, 255));

	EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));

	/* Download the raw data into intended locations */
	{
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));

		Load_RawDataFromfile(SAMAPP_Abs_Base_Dial_L1, SAMAPP_Abs_BaseDial_Size146x146_L1, 0, L4, 146, 146, 146);
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 255, 255));
		Load_RawDataFromfile(SAMAPP_Abs_Meter_Dial_rgb, SAMAPP_Abs_MeterDial_Size146x146_rgb, 1, RGB565, 146 * 2, 146, 146);

	}

	EVE_Cmd_wr32(s_pHalContext, DISPLAY());
	EVE_CoCmd_swap(s_pHalContext);

	/* Wait till coprocessor completes the operation */
	EVE_Cmd_waitFlush(s_pHalContext);

	/*asiign track to the black dot*/
	EVE_CoCmd_track(s_pHalContext, CircleX, CircleY, DIAL, DIAL, 1);
	G1 = 0x1E1E1E;
	G2 = 0x999999;

	EVE_Hal_wr8(s_pHalContext, REG_PLAY, 1);

	do {
		Read_tag = EVE_Hal_rd8(s_pHalContext, REG_TOUCH_TAG);
		Touch = EVE_Hal_rd16(s_pHalContext, REG_TOUCH_SCREEN_XY + 2);
		RotaryTag = EVE_Hal_rd32(s_pHalContext, REG_TRACKER);
		EVE_CoCmd_dlStart(s_pHalContext);
		EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
		EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_RGB(0, 0, 0));

		EVE_Cmd_wr32(s_pHalContext, SCISSOR_XY(0, 0));

		EVE_CoCmd_gradient(s_pHalContext, 0, 0, G1, Sx1, Sy1, G2);

		EVE_Cmd_wr32(s_pHalContext, SCISSOR_SIZE(Sx1, Sy1));

		EVE_Cmd_wr32(s_pHalContext, SCISSOR_SIZE(s_pHalContext->Width, s_pHalContext->Height));

		/* Assign Stencil Value to get the triangle shape */
		EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(0, 0, 0, 1));//only alpha is enabled,so that no color is used for strip
		EVE_Cmd_wr32(s_pHalContext, LINE_WIDTH(1 * 16));
		EVE_Cmd_wr32(s_pHalContext, STENCIL_OP(INCR, INVERT));
		EVE_Cmd_wr32(s_pHalContext, STENCIL_FUNC(EQUAL, 0, 255));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(EDGE_STRIP_B));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(136, 136, 136));

		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(Tx1 * 16, s_pHalContext->Height * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(Tx2 * 16, Ty2 * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(Tx3 * 16, s_pHalContext->Height * 16));

		EVE_Cmd_wr32(s_pHalContext, STENCIL_OP(KEEP, INCR));
		EVE_Cmd_wr32(s_pHalContext, STENCIL_FUNC(NOTEQUAL, 255, 255));
		EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(1, 1, 1, 1));//enable all colors

		/* Outer Black circle*/
		/* Based on stencil update Points color,leaving the triangular area*/

		EVE_Cmd_wr32(s_pHalContext, BEGIN(FTPOINTS));
		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE((OuterCircleRadius) * 16));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));

		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(((OuterCircleRadius - 3) * 16)));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(15, 77, 116));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));

		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(((OuterCircleRadius - 5) * 16)));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(54, 115, 162));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));

		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(((OuterCircleRadius - 7) * 16)));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(76, 195, 225));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));

		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(((OuterCircleRadius - 9) * 16)));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(89, 211, 232));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));

		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(((OuterCircleRadius - 11) * 16)));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(123, 248, 250));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));

		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE((OuterCircleRadius - 17) * 16));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(89, 211, 232));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));

		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE((OuterCircleRadius - 18) * 16));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(76, 195, 225));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));

		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE((OuterCircleRadius - 19) * 16));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(54, 115, 162));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));

		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE((OuterCircleRadius - 20) * 16));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(15, 77, 116));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));

		/*Managing the track*/
		if ((Touch != NoTouch))
		{
			if ((RotaryTag & 0xff) == 1)
			{
				CurrTh = (int16_t)((RotaryTag >> 16) & 0xffff);
				/* BaseTrackVal is the rotary track value of the particular white circle that is moving */
				if ((adjusting != 0))
				{
					BaseTrackValSign += (int16_t)(CurrTh - PrevTh);
				}
				PrevTh = CurrTh;
				adjusting = (RotaryTag & 0xff);
			}
		}

		/* alpha bend for strips */

		EVE_Cmd_wr32(s_pHalContext, COLOR_A(255));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0xff, 0xff, 0xff));
		EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(0, 0, 0, 1));
		EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(FTPOINTS));
		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE((OuterCircleRadius - 2) * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(CircleX, CircleY, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(1, 1, 1, 1));

		EVE_Cmd_wr32(s_pHalContext, BLEND_FUNC(DST_ALPHA, ONE_MINUS_DST_ALPHA));

		/* Parameters for Strips */
		EVE_Cmd_wr32(s_pHalContext, LINE_WIDTH(2 * 16));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(30, 30, 30));
		Delta = BaseTrackValInit - BaseTrackValSign;

		if ((prevtag == 1) && (Read_tag == 1) && (Touch != -32768))
		{
			PrevBaseVal = PrevBaseVal - Delta;
			MoveDeg = (uint16_t)(PrevBaseVal / 182);

		}

		if (PrevBaseVal > 60294) PrevBaseVal = 60294;
		if (PrevBaseVal < 0)PrevBaseVal = 0;
		if (MoveDeg > 330) MoveDeg = 330;
		if (MoveDeg < 30) MoveDeg = 31;

		if (MoveDeg <= 90)
		{
			/* First Left edge strip */
			EVE_Cmd_wr32(s_pHalContext, BEGIN(EDGE_STRIP_L));
			angle = 2 * Math_Da(MoveDeg, -180);
			Math_Polarxy((OuterCircleRadius - 2), angle, &X1, &Y1, CircleX, CircleY);
			EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));
			EVE_Cmd_wr32(s_pHalContext, VERTEX2F(X1, Y1));
		}

		if (MoveDeg <= 180)
		{
			/* Second Above edge strip */
			EVE_Cmd_wr32(s_pHalContext, BEGIN(EDGE_STRIP_A));
			if (MoveDeg <= 90)
			{
				angle = 2 * Math_Da(90, -180);
				Math_Polarxy((OuterCircleRadius - 2), angle, &X1, &Y1, CircleX, CircleY);
			}
			else if (MoveDeg > 90)
			{
				angle = 2 * Math_Da(MoveDeg, -180);
				Math_Polarxy((OuterCircleRadius - 2), angle, &X1, &Y1, CircleX, CircleY);
			}
			EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));
			EVE_Cmd_wr32(s_pHalContext, VERTEX2F(X1, Y1));

		}
		if (MoveDeg <= 270)
		{
			/* Third Above edge strip */
			EVE_Cmd_wr32(s_pHalContext, BEGIN(EDGE_STRIP_R));
			if (MoveDeg <= 180)
			{
				angle = 2 * Math_Da(180, -180);
				Math_Polarxy((OuterCircleRadius - 2), angle, &X1, &Y1, CircleX, CircleY);
			}
			else if (MoveDeg > 180)
			{
				angle = 2 * Math_Da(MoveDeg, -180);
				Math_Polarxy((OuterCircleRadius - 2), angle, &X1, &Y1, CircleX, CircleY);
			}
			EVE_Cmd_wr32(s_pHalContext, VERTEX2F(X1, Y1));
			EVE_Cmd_wr32(s_pHalContext, VERTEX2F(CircleX * 16, CircleY * 16));
		}

		if (MoveDeg <= 330)
		{
			/* Fourth Above edge strip */
			EVE_Cmd_wr32(s_pHalContext, BEGIN(EDGE_STRIP_B));
			if (MoveDeg <= 270)
			{
				angle = 2 * Math_Da(270, -180);
				Math_Polarxy((OuterCircleRadius - 1), angle, &X1, &Y1, CircleX, CircleY);
			}
			else if (MoveDeg > 270)
			{
				angle = 2 * Math_Da(MoveDeg, -180);
				Math_Polarxy((OuterCircleRadius - 1), angle, &X1, &Y1, CircleX, CircleY);
			}
			EVE_Cmd_wr32(s_pHalContext, VERTEX2F((CircleX) * 16, CircleY * 16));
			EVE_Cmd_wr32(s_pHalContext, VERTEX2F(X1, Y1));
		}

		EVE_Cmd_wr32(s_pHalContext, BLEND_FUNC(SRC_ALPHA, ONE_MINUS_SRC_ALPHA));

		EVE_Cmd_wr32(s_pHalContext, STENCIL_OP(KEEP, KEEP));
		EVE_Cmd_wr32(s_pHalContext, STENCIL_FUNC(ALWAYS, 0, 255));

		/*alpha blend for rgb bitmap*/
		EVE_Cmd_wr32(s_pHalContext, COLOR_A(0xff));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0xff, 0xff, 0xff));
		EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(0, 0, 0, 1));
		EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(FTPOINTS));
		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE((146 / 2) * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(CircleX, CircleY, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(1, 1, 1, 1));

		EVE_Cmd_wr32(s_pHalContext, BLEND_FUNC(DST_ALPHA, ONE_MINUS_DST_ALPHA));

		/* Meter Dial bitmap placement */
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(CircleX - 146 / 2, CircleY - 146 / 2, 1, 0));
		EVE_Cmd_wr32(s_pHalContext, BLEND_FUNC(SRC_ALPHA, ONE_MINUS_SRC_ALPHA));
		EVE_Cmd_wr32(s_pHalContext, END());

		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
		EVE_Cmd_wr32(s_pHalContext, COLOR_A(20));
		EVE_Cmd_wr32(s_pHalContext, TAG(1));

		/* Alpha Meter Dial bitmap placement & rotation */
		EVE_Cmd_wr32(s_pHalContext, SAVE_CONTEXT());
		EVE_CoCmd_loadIdentity(s_pHalContext);
		EVE_CoCmd_translate(s_pHalContext, F16(73), F16(73));
		EVE_CoCmd_rotate(s_pHalContext, MoveDeg * 65536 / 360);
		EVE_CoCmd_translate(s_pHalContext, F16(-(73)), F16(-(73)));
		EVE_CoCmd_setMatrix(s_pHalContext);
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(CircleX - 146 / 2, CircleY - 146 / 2, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, RESTORE_CONTEXT());

		/* restore default alpha value */
		EVE_Cmd_wr32(s_pHalContext, COLOR_A(255));

		EVE_CoCmd_fgColor(s_pHalContext, 0x59D3E8);
		EVE_CoCmd_gradColor(s_pHalContext, 0x7BF8FA);

		EVE_CoCmd_button(s_pHalContext, (ScreenWidthMid - 40), ButY, 80, ButWid, 28, 0, "");// placement of button

		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));

		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));//color of number(text) color

		if ((MoveDeg >= 30) && (MoveDeg <= 330))
		{
			Volume = PrevBaseVal >> 8;
			EVE_Cmd_wr32(s_pHalContext, BEGIN(LINES));
			EVE_Cmd_wr32(s_pHalContext, LINE_WIDTH(1 * 16));
			EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(30, 30, 30));
			angle = Math_Da(MoveDeg, -180) * 2;
			Math_Polarxy((OuterCircleRadius - 3), angle, &X1, &Y1, CircleX, CircleY);
			Math_Polarxy(74, angle, &X2, &Y2, CircleX, CircleY);
			EVE_Cmd_wr32(s_pHalContext, VERTEX2F(X1, Y1));
			EVE_Cmd_wr32(s_pHalContext, VERTEX2F(X2, Y2));
		}

		EVE_CoCmd_number(s_pHalContext, (ScreenWidthMid), NoY, 28, OPT_CENTER, Volume);
		EVE_Hal_wr8(s_pHalContext, REG_VOL_SOUND, Volume);
		EVE_Cmd_wr32(s_pHalContext, DISPLAY());

		EVE_CoCmd_swap(s_pHalContext);

		/* Wait till coprocessor completes the operation */
		EVE_Cmd_waitFlush(s_pHalContext);
		prevtag = Read_tag;
		BaseTrackValInit = BaseTrackValSign;
		EVE_sleep(30);
	} while (1);
}

#endif

#ifdef Relative_Dial

void Relative_Meter_Dial()
{
	uint8_t i = 0, j = 0, k = 0, Read_tag, prevtag = 0, currtag;
	int16_t ScreenWidthMid, ScreenHeightMid, NextTh = 0, Touch, drag = 0;
	int16_t OuterCircleRadius, PrevValDeg = 0, CircleX, CircleY, ButY, ButWid, NoY, Delta, G1, G2, Sx1, Sy1;

	int32_t PrevTh = 0, CurrTh, adjusting = 0, BaseTrackValSign = 0, Vx1 = 0, Vy1 = 0, Vx2 = 0, Vy2 = 0, Volume = 0, PrevBaseTrackVal = 0, X2, Y2;

	uint32_t RotaryTag = 0;

	int32_t change;
	int32_t sx, storesx, storepresx;

	ScreenWidthMid = s_pHalContext->Width / 2;
	ScreenHeightMid = s_pHalContext->Height / 2;

	PrevValDeg = 0;
	Sx1 = Sy1 = 180;

#ifdef DISPLAY_RESOLUTION_WQVGA
	Sx1 = Sy1 = 240;
	OuterCircleRadius = ScreenWidthMid - 144;
	CircleX = ScreenWidthMid;
	CircleY = ScreenHeightMid;
	ButY = s_pHalContext->Height - 262;
	ButWid = 30;
	NoY = (s_pHalContext->Height - 262 + 30 / 2);
#endif
#if defined(DISPLAY_RESOLUTION_QVGA) || defined(DISPLAY_RESOLUTION_HVGA_PORTRAIT)
	Sx1 = Sy1 = 130;
	ButY = 1;
	ButWid = 23;
	NoY = 12;
	OuterCircleRadius = ScreenWidthMid - 64;
	CircleX = ScreenWidthMid - 70 + 146 / 2;
	CircleY = ScreenHeightMid - 67 + 146 / 2;

#endif

#ifdef DISPLAY_RESOLUTION_WVGA
	Sx1 = Sy1 = 400;
	OuterCircleRadius = ScreenWidthMid - 100;
	CircleX = ScreenWidthMid;
	CircleY = ScreenHeightMid;
	ButY = s_pHalContext->Height - 460;
	ButWid = 150;
	NoY = (s_pHalContext->Height - 460 + 30 / 2);
#endif

	EVE_Cmd_wr32(s_pHalContext, CMD_MEMSET);
	EVE_Cmd_wr32(s_pHalContext, 0L);//starting address of memset
	EVE_Cmd_wr32(s_pHalContext, 55L);//value of memset
	EVE_Cmd_wr32(s_pHalContext, 256 * 1024);

	/* set the vol to MAX */
	EVE_Hal_wr8(s_pHalContext, REG_VOL_SOUND, 0);
	/* Play carousel*/
	EVE_Hal_wr16(s_pHalContext, REG_SOUND, 8);

	/* Construction of starting screen shot, assign all the bitmap handles here */
	EVE_Cmd_wr32(s_pHalContext, CMD_DLSTART);
	EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_RGB(255, 255, 255));

	EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));

	/* Download the raw data into intended locations */
	{
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));
		Load_RawDataFromfile(SAMAPP_Relative_Meter_Dial_rgb, SAMAPP_Relative_MeterDial_Size146x146_rgb, 1, RGB565, 146 * 2, 146, 146);

		Load_RawDataFromfile(SAMAPP_Abs_Base_Dial_L1, SAMAPP_Abs_BaseDial_Size146x146_L1, 0, L4, 146, 146, 146);
	}

	EVE_Cmd_wr32(s_pHalContext, DISPLAY());
	EVE_CoCmd_swap(s_pHalContext);

	/* Wait till coprocessor completes the operation */
	EVE_Cmd_waitFlush(s_pHalContext);

	/*asiign track to the bitmap*/
	EVE_CoCmd_track(s_pHalContext, (CircleX)+40, CircleY, DIAL, DIAL, 1);

	EVE_Hal_wr8(s_pHalContext, REG_PLAY, 1);

	do {
		EVE_Cmd_wr32(s_pHalContext, CLEAR_TAG(255));
		Read_tag = EVE_Hal_rd8(s_pHalContext, REG_TOUCH_TAG);
		Touch = EVE_Hal_rd16(s_pHalContext, REG_TOUCH_SCREEN_XY + 2);
		RotaryTag = EVE_Hal_rd32(s_pHalContext, REG_TRACKER);
		EVE_CoCmd_dlStart(s_pHalContext);
		EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
		EVE_Cmd_wr32(s_pHalContext, CLEAR_COLOR_RGB(0, 0, 0));

		EVE_Cmd_wr32(s_pHalContext, SCISSOR_XY(0, 0));

#if defined(DISPLAY_RESOLUTION_WQVGA) 
		EVE_CoCmd_gradient(s_pHalContext, 0, s_pHalContext->Height, 0x1E1E1E, ScreenWidthMid - 60/*180*/, ScreenHeightMid - 21/*115*/, 0x999999);

		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(LINES));

		EVE_Cmd_wr32(s_pHalContext, VERTEX2F((s_pHalContext->Width - 2) * 16, (s_pHalContext->Height) * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(ScreenWidthMid * 16, 0 * 16));

#elif defined(DISPLAY_RESOLUTION_QVGA)
		EVE_CoCmd_gradient(s_pHalContext, 0, s_pHalContext->Height, 0x1E1E1E, 140/*120*/, 140/*101*/, 0x999999);

		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(LINES));

		EVE_Cmd_wr32(s_pHalContext, VERTEX2F((s_pHalContext->Width - 2) * 16, (s_pHalContext->Height) * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(146 * 16, 0 * 16));
#elif defined(DISPLAY_RESOLUTION_HVGA_PORTRAIT)
		EVE_CoCmd_gradient(s_pHalContext, 0, s_pHalContext->Height, 0x1E1E1E, ScreenWidthMid - 60, ScreenHeightMid - 21, 0x999999);

		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));

		EVE_Cmd_wr32(s_pHalContext, BEGIN(LINES));

		EVE_Cmd_wr32(s_pHalContext, VERTEX2F((0) * 16, (30) * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(s_pHalContext->Width * 16, 154 * 16));
#elif defined(DISPLAY_RESOLUTION_WVGA)
		EVE_CoCmd_gradient(s_pHalContext, 0, s_pHalContext->Height, 0x1E1E1E, 300/*60*/, 217/*21*/, 0x999999);

		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(LINES));

		EVE_Cmd_wr32(s_pHalContext, VERTEX2F((s_pHalContext->Width - 2) * 16, (s_pHalContext->Height) * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(ScreenWidthMid * 16, 0 * 16));
#endif

		sx = 0;
		currtag = (RotaryTag & 0xff);
		if (1 == currtag)
		{
			sx = RotaryTag >> 16;
		}

		if ((prevtag == 1) && (1 == currtag))
		{
			scroller.vel = (-scroller.dragprev + sx);

			if ((scroller.vel > 32000))
			{
				scroller.vel -= 65536;
			}

			else if (scroller.vel < -32768)
			{
				scroller.vel += 65536;
			}
			scroller.vel <<= 4;

			storepresx = scroller.dragprev;
			storesx = sx;
		}
		else
		{
			change = MAX(1, abs(scroller.vel) >> 5);
			if (scroller.vel < 0)
				scroller.vel += change;

			if (scroller.vel > 0)
				scroller.vel -= change;

		}
		prevtag = currtag;
		scroller.dragprev = sx;
		scroller.base += scroller.vel;

		drag = scroller.base >> 4;

		/*Managing the track*/
		{
			if ((1 == currtag))
			{
				CurrTh = sx;

				if ((adjusting != 0))
				{
					PrevBaseTrackVal = (CurrTh - PrevTh);

					if ((PrevBaseTrackVal > 32000))
					{
						PrevBaseTrackVal -= 65536;
					}
					else if (PrevBaseTrackVal < -32768)
					{
						PrevBaseTrackVal += 65536;
					}
					BaseTrackValSign += PrevBaseTrackVal;
					BaseTrackVal += PrevBaseTrackVal;

				}

				adjusting = (RotaryTag & 0xff);
				PrevTh = CurrTh;
				NextTh = (uint16_t)(BaseTrackValSign / 182);
				PrevValDeg = NextTh;

			}

			else if ((0 != scroller.vel))
			{
				BaseTrackValSign += (scroller.vel >> 4); //original copy
				BaseTrackVal += (scroller.vel >> 4);;
				NextTh = (uint16_t)(BaseTrackValSign / 182);
				PrevValDeg = NextTh;
				adjusting = 0;
			}
			else
			{
				adjusting = 0;
			}

		}

		/*alpha blend for rgb bitmap*/

		EVE_Cmd_wr32(s_pHalContext, COLOR_A(0xff));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0xff, 0xff, 0xff));
		EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(0, 0, 0, 1));
		EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(FTPOINTS));
		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE((146 / 2) * 16));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(CircleX + 40, CircleY, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, COLOR_MASK(1, 1, 1, 1));

		EVE_Cmd_wr32(s_pHalContext, BLEND_FUNC(DST_ALPHA, ONE_MINUS_DST_ALPHA));

		/* Meter Dial bitmap placement */
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));

		EVE_Cmd_wr32(s_pHalContext, VERTEX2II((CircleX - 146 / 2) + 40, CircleY - 146 / 2, 1, 0));

		EVE_Cmd_wr32(s_pHalContext, BLEND_FUNC(SRC_ALPHA, ONE_MINUS_SRC_ALPHA));

		Delta = BaseTrackValSign - BaseTrackValInit;

		EVE_Cmd_wr32(s_pHalContext, COLOR_A(20));

		/* Alpha Meter Dial bitmap placement & rotation */
		EVE_Cmd_wr32(s_pHalContext, SAVE_CONTEXT());
		EVE_Cmd_wr32(s_pHalContext, TAG_MASK(1));

		EVE_CoCmd_loadIdentity(s_pHalContext);
		EVE_CoCmd_translate(s_pHalContext, F16(73), F16(73));
		EVE_CoCmd_rotate(s_pHalContext, PrevValDeg * 65536 / 360);
		EVE_CoCmd_translate(s_pHalContext, F16(-(73)), F16(-(73)));
		EVE_CoCmd_setMatrix(s_pHalContext);
		EVE_Cmd_wr32(s_pHalContext, TAG(1));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II((CircleX - 146 / 2) + 40, CircleY - 146 / 2, 0, 0));

		EVE_Cmd_wr32(s_pHalContext, RESTORE_CONTEXT());
		EVE_Cmd_wr32(s_pHalContext, TAG(1));
		/* Draw points around the bitmap*/
		EVE_Cmd_wr32(s_pHalContext, COLOR_A(255));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(FTPOINTS));
		EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(5 * 16));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(227, 249, 20));
		angle = Math_Da(PrevValDeg, -180) * 2;
		Math_Polarxy(55, angle, &X2, &Y2, CircleX + 40, CircleY);

		EVE_Cmd_wr32(s_pHalContext, VERTEX2F(X2, Y2));
		EVE_Cmd_wr32(s_pHalContext, TAG_MASK(0));

		{
#if defined(DISPLAY_RESOLUTION_HVGA_PORTRAIT)
			uint8_t statusButtonH = 20, statusButtonW = 30, buttonsPerRow = 6, rowGap = 10, curRow = 0;
			uint16_t edgeOffset = 30, startingX = 10, startingY = s_pHalContext->Height - statusButtonH - edgeOffset;

			uint16_t buttonVerticalGap = (s_pHalContext->Height - edgeOffset * 2 - buttonsPerRow * statusButtonH) / (buttonsPerRow - 1);

			Vy1 = startingY;
#endif

			for (k = 0; k <= 11; k++)
			{
#if defined(DISPLAY_RESOLUTION_HVGA_PORTRAIT)
				Vx1 = startingX + (curRow * statusButtonW) + (curRow * rowGap);

#else
				Vy1 = 200;
				if (k == 0)/* First button Vertices*/
				{
					Vx1 = 10; Vy1 = 200;
				}
				else if ((k == 1) || (k == 2) || (k == 3))/* Second,third,fourth button Vertices*/
				{
					Vx1 = 10; Vy1 = Vy1 - (k) * 60;
				}
				else if (k == 4)/* fifth button Vertices*/
				{
					Vx1 = 40; Vy1 = 200;
				}
				else if ((k == 5) | (k == 6) | (k == 7))/* sixth,seventh,eighth button Vertices*/
				{
					Vx1 = 40; Vy1 = Vy1 - (k - 4) * 60;
				}
				else if (k == 8)/* ninth button Vertices*/
				{
					Vx1 = 70; Vy1 = 200;
				}
				else if ((k == 9) | (k == 10) | (k == 11))/* tenth,eleven,twelth button Vertices*/
				{
					Vx1 = 70; Vy1 = Vy1 - (k - 8) * 60;
				}
#endif			

				if (BaseTrackVal < 11 * 65535)
				{
					/* Rotary tag when within range of particular index, highlight only the indexed button in yellow*/
					if (((BaseTrackVal >= k * 65535) && (BaseTrackVal < (k + 1) * 65535)))//|| ((BaseTrackValSign<0)&&(k==0)))
					{
						EVE_CoCmd_fgColor(s_pHalContext, 0xE3F914);//yellow
						EVE_CoCmd_button(s_pHalContext, Vx1, Vy1, 20, 30, 16, 0, "");
					}
					else/* Color the non indexed buttons in grey color*/
					{
						EVE_CoCmd_fgColor(s_pHalContext, 0x787878);//grey
						EVE_CoCmd_button(s_pHalContext, Vx1, Vy1, 20, 30, 16, 0, "");
					}
				}

				/*Limit track value when its greater than 12th button(11th index) and also highlight twelth button*/
				else if ((BaseTrackVal >= 11 * 65535))
				{
					BaseTrackVal = 11 * 65535;

					if (k < 11)
					{
						EVE_CoCmd_fgColor(s_pHalContext, 0x787878);//grey
					}
					else
					{
						EVE_CoCmd_fgColor(s_pHalContext, 0xE3F914);//yellow					
					}
					EVE_CoCmd_button(s_pHalContext, Vx1, Vy1, 20, 30, 16, 0, "");
				}

				/*Limit track value when its negative and also highlight first button*/
				if ((BaseTrackVal < 0) && (k == 0))
				{
					if ((Delta > 0))
						BaseTrackVal = 0;
					EVE_CoCmd_fgColor(s_pHalContext, 0xE3F914);//yellow	
					EVE_CoCmd_button(s_pHalContext, Vx1, Vy1, 20, 30, 16, 0, "");
				}

#if defined(DISPLAY_RESOLUTION_HVGA_PORTRAIT)
				Vy1 -= (statusButtonH + buttonVerticalGap);

				if (Vy1 < edgeOffset) {
					curRow++;
					Vy1 = startingY;
				}
#endif
			}

		}

		/* play volume*/

		if (BaseTrackVal < 0)Volume = 0;
		else
			Volume = BaseTrackVal >> 12;

		EVE_Hal_wr8(s_pHalContext, REG_VOL_SOUND, Volume);

		EVE_Cmd_wr32(s_pHalContext, DISPLAY());
		EVE_CoCmd_swap(s_pHalContext);
		/* Wait till coprocessor completes the operation */
		EVE_Cmd_waitFlush(s_pHalContext);
		BaseTrackValInit = BaseTrackValSign;
		prevtag = Read_tag;
		EVE_sleep(30);
	} while (1);
}
#endif

void DemoMeter() {
#ifdef Absolute_Dial
	Abs_Meter_Dial();
#endif
#ifdef Relative_Dial
	Relative_Meter_Dial();
#endif /*Relative_Dial*/
}

