/**
 * @file DemoMetaballs.c
 * @brief Balls demo
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
#include "EVE_CoCmd.h"
#include "Common.h"
#include "App.h"

static EVE_HalContext *s_pHalContext;

typedef signed char      schar8_t;

static int16_t v()
{
#if defined(DISPLAY_RESOLUTION_WVGA) || defined( DISPLAY_RESOLUTION_HVGA_PORTRAIT)
	return random(50) - 32;

#else
	return random(80) - 32;
#endif
}

static uint8_t istouch()
{
	return !(EVE_Hal_rd16(s_pHalContext, REG_TOUCH_RAW_XY) & 0x8000);
}

void DemoMetaballs(EVE_HalContext* pHalContext) {
	s_pHalContext = pHalContext;
	uint8_t w = 31, h = 18, numBlobs = 80,
		*recip, fadein, f, temp[31];

	int32_t
		centerx = (16 * 16 * (w / 2)),
		centery = (16 * 16 * (h / 2)),
		touching,
		recipsz = (w*w + h*h) / 4 + 1,
		tval, tval1, tval2,
		sx, sy,
		VEL,
		m, d, bx, by, dx, dy;
	struct
	{
		int16_t x, y;
		schar8_t dx, dy;
	} blobs[80];

	for (tval = 0; tval<numBlobs; ++tval)
	{
		blobs[tval].x = random(16 * s_pHalContext->Width);
		blobs[tval].y = random(16 * s_pHalContext->Height);
		blobs[tval].dx = (schar8_t)v();
		blobs[tval].dy = (schar8_t)v();
	}

	recip = (uint8_t*)malloc(recipsz);
	for (tval = 0; tval < recipsz; tval++)
	{
		if (tval == 0)
		{
			recip[tval] = 200;
		}
		else
		{
			if (s_pHalContext->Width == 480) {
				recip[tval] = MIN(200, (s_pHalContext->Width * 10) / (tval));
			}
			else {
				recip[tval] = MIN(200, (s_pHalContext->Width * 10) / (4 * tval));
			}
			
		}
	}
	fadein = 255;
	f = 0;

	while (1)
	{
		{
			touching = istouch();
			if (touching)
			{
				sx = EVE_Hal_rd16(s_pHalContext, REG_TOUCH_SCREEN_XY + 2);
				sy = EVE_Hal_rd16(s_pHalContext, REG_TOUCH_SCREEN_XY);
				centerx = 16 * sx;
				centery = 16 * sy;
			}
			else
			{
				centerx = s_pHalContext->Width * 16 / 2;
				centery = s_pHalContext->Height * 16 / 2;
			}
		}
		VEL = touching ? 8 : 2;
		for (tval = 0; tval<numBlobs; ++tval)
		{
			if (blobs[tval].x < centerx)   blobs[tval].dx += VEL;    else
				blobs[tval].dx -= VEL;
			if (blobs[tval].y < centery)   blobs[tval].dy += VEL;   else
				blobs[tval].dy -= VEL;
			blobs[tval].x += blobs[tval].dx << 3;
			blobs[tval].y += blobs[tval].dy << 3;
		}
		blobs[random(numBlobs)].dx = (schar8_t)v();
		blobs[random(numBlobs)].dy = (schar8_t)v();
		for (tval1 = 0; tval1 < h; tval1++)
		{
			for (tval2 = 0; tval2 < w; tval2++)
			{
				m = fadein;
				for (tval = 0; tval < 3; tval++)
				{
					bx = blobs[tval].x >> 8;
					by = blobs[tval].y >> 8;
					dx = bx - tval2;
					dy = by - tval1;
					d = SQ(dx) + SQ(dy);
					m += recip[MIN(d >> 2, recipsz - 1)];
				}
				temp[tval2] = MIN(m, 255);
			}
			EVE_Hal_wrMem(s_pHalContext, (f << 12) + (tval1 << 6), temp, w);
		}

		EVE_CoCmd_dlStart(s_pHalContext);        // start
		EVE_Cmd_wr32(s_pHalContext, CLEAR(1, 1, 1));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SOURCE(f << 12));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_LAYOUT(L8, 64, 64));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE(BILINEAR, BORDER, BORDER, s_pHalContext->Width, s_pHalContext->Height));
		if (EVE_CHIPID >= EVE_FT810)
		{
			EVE_Cmd_wr32(s_pHalContext, BITMAP_SIZE_H(s_pHalContext->Width >> 9, s_pHalContext->Height >> 9));
		}
		EVE_Cmd_wr32(s_pHalContext, SAVE_CONTEXT());
		EVE_Cmd_wr32(s_pHalContext, BITMAP_TRANSFORM_A(0x100 / 64));
		EVE_Cmd_wr32(s_pHalContext, BITMAP_TRANSFORM_E(0x100 / 64));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(BITMAPS));

		EVE_Cmd_wr32(s_pHalContext, BLEND_FUNC(SRC_ALPHA, ZERO));
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(0, 0, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, BLEND_FUNC(SRC_ALPHA, ONE));

		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(255, 255, 0));
		EVE_Cmd_wr32(s_pHalContext, VERTEX2II(0, 0, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, RESTORE_CONTEXT());
		EVE_Cmd_wr32(s_pHalContext, COLOR_RGB(0, 0, 0));
		EVE_Cmd_wr32(s_pHalContext, BEGIN(FTPOINTS));
		for (tval = 3; tval < numBlobs; tval++)
		{
			EVE_Cmd_wr32(s_pHalContext, POINT_SIZE(3 * tval));
			EVE_Cmd_wr32(s_pHalContext, VERTEX2F(blobs[tval].x, blobs[tval].y));
		}
		EVE_Cmd_wr32(s_pHalContext, DISPLAY());
		EVE_CoCmd_swap(s_pHalContext);
		EVE_Cmd_waitFlush(s_pHalContext);
		fadein = MAX(fadein - 3, 1);
		f = (f + 1) & 3;
	}
}

