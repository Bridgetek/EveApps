/**
 * @file Event.c
 * @brief User event
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

#include "Event.h"
#include <time.h>
#define MIN_MOVE         15
#define MIN_TOUCH        14

void getTouchEvent(EVE_HalContext *pHalContext, int *x, int *y, int8_t *isTouch)
{
	*x = Gpu_Hal_Rd16(pHalContext, REG_TOUCH_SCREEN_XY);
	*y = Gpu_Hal_Rd16(pHalContext, REG_TOUCH_SCREEN_XY + 2);

	*isTouch = !(Gpu_Hal_Rd16(pHalContext, REG_TOUCH_RAW_XY) & 0x8000);
}

uchar8_t is_touch(Gpu_Hal_Context_t *pHalContext) {
	uchar8_t ret = !(Gpu_Hal_Rd16(pHalContext, REG_TOUCH_RAW_XY) & 0x8000);
	return ret;
}
