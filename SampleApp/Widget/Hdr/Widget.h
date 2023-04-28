/**
 * @file Widget.h
 * @brief Sample usage of some widgets
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

#ifndef APP_H_
#define APP_H_

#include "platform.h"

// Path to UI assets Folder
#if defined(MSVC_PLATFORM) || defined(BT8XXEMU_PLATFORM)
#define TEST_DIR                            "..\\..\\..\\Test\\"
#else
#define TEST_DIR                            "/EveApps/SampleApp/Widget/Test"
#endif

#define GET_CALIBRATION                     1

/* sample app structure definitions */
typedef struct SAMAPP_Bitmap_header
{
	uint8_t Format;
	int16_t Width;
	int16_t Height;
	int16_t Stride;
	int32_t Arrayoffset;
}SAMAPP_Bitmap_header_t;

#endif /* APP_H_ */
