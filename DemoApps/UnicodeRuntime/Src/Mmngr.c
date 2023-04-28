/**
 * @file Mmngr.c
 * @brief Memory manager
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

#include "Mmngr.h"
#include "Common.h"

static unsigned int memUsed = 0;
char* memget(int size) {
	char *ptr = (char*)malloc(size);
	if (!ptr) {
		APP_ERR("Malloc failed for %d bytes, total used = %u ", size, memUsed);
		return 0;
	}

	memUsed += size;
	return ptr;
}
int memfree(char *ptr, int size) {
	free(ptr);
	memUsed -= size;
	return APP_OK;
}
int dumpMem(char *start, int size) {
#if APPDEBUG
	const int bytesPerLine = 64;
	const int group = 1;
	int t = 0;
	char *c = (char*)start;
	unsigned int sum = 0;

	for (int i = 0; i < size;) {
		t = 0;
		for (int j = 0; j < (bytesPerLine) && i < size; j++) {
			unsigned char x = c[i];
			sum += x;
			printf("%02x", x);
			i++;
			t++;
			if (t == group) {
				t = 0;
				printf(" ");
			}
		}
		printf("\n");
	}
#endif
	return APP_OK;
}
