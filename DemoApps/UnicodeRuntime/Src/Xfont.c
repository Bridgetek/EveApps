/**
 * @file Xfont.c
 * @brief Xfont processor
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

#include "Xfont.h"
#include "Common.h"

int getGlyOffset(char* xf, unsigned long cp) {
	int noc = XF_NUMBER_OF_CHARACTERS(xf); // number_of_characters
	unsigned int* gPtr = XF_GPTR(xf);
	uint32_t bytes_per_glyph = XF_LAYOUT_WIDTH(xf) * XF_LAYOUT_HEIGHT(xf);
	int ret =  gPtr[cp / 128] + bytes_per_glyph * (cp % 128);

	return ret;
}
unsigned char getWidth(char *xf, unsigned long cp) {
	unsigned int* wPtr = XF_WPTR(xf);
	unsigned int offset = wPtr[cp / 128] + (cp % 128);
	char *widthdata = xf;

	return (unsigned char) widthdata[offset];
}

int parseXFONT(char *fileBuffer, XFONTEXTENDED *xf) {
	int *header = (int*)fileBuffer;
	int offset = 0;

	xf = (XFONTEXTENDED*)fileBuffer;

	xf->signature = header[offset++];
	xf->size = header[offset++];
	xf->format = header[offset++];
	xf->swizzle = header[offset++];
	xf->layout_width = header[offset++];
	xf->layout_height = header[offset++];
	xf->pixel_width = header[offset++];
	xf->pixel_height = header[offset++];
	xf->start_of_Graphic_data = header[offset++];
	xf->number_of_characters = header[offset++];

	xf->gPtrOffset = offset * 4;
	xf->gPtrSize = 4 * (xf->number_of_characters / 128);

	xf->wPtrOffset = xf->gPtrOffset + xf->gPtrSize;
	xf->wPtrSize = 4 * (xf->number_of_characters / 128);

	xf->widthdataOffset = xf->wPtrOffset + xf->wPtrSize;
	xf->widthdataSize = xf->number_of_characters;

	APP_DBG("Getting gptr data from offset %d, %d bytes", xf->gPtrOffset, xf->gPtrSize);
	xf->gptr = (int*)&fileBuffer[xf->gPtrOffset];

	APP_DBG("Getting wptr data from offset %d, %d bytes", xf->wPtrOffset, xf->wPtrSize);
	xf->wptr = (int*)&fileBuffer[xf->wPtrOffset];

	APP_DBG("widthdataOffset: %d", xf->widthdataOffset);
	xf->width = (int*)&fileBuffer[xf->widthdataOffset];

	return APP_OK;
}
