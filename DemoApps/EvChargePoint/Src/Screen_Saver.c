/**
 * @file Screen_Saver.c
 * @brief Screen saver page
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

#include "Screen_Saver.h"
#include "Def.h"
#include "App.h"

// Global variables
extern Gpu_Hal_Context_t* g_pHalContext;
extern E_LANG g_language;

static uint32_t frame_addr = SS_FLASH_ADDR_FRAME_0;
static uint32_t frame_index = 0;
static int16_t ani_color = 0;
static int8_t ani_color_step = 5;
extern const char * s_pleaseTouch;

#define ANI_STEP        (10)

void ss_init() {
    ani_color = 0;
    ani_color_step = ANI_STEP;
}

void load_animation() {
    Gpu_CoCmd_FlashRead(g_pHalContext, RAM_DL_SIZE - SS_FRAME_SIZE, frame_addr, SS_FRAME_SIZE);

    if (g_language == LANG_CN) {
        Gpu_CoCmd_SetFont2(g_pHalContext, HF_BOTTOM, FontBottomCH.xf_addr - FontBegin.xf_addr, 0);
    } else {
        Gpu_CoCmd_SetFont2(g_pHalContext, HF_BOTTOM, FontBottom.xf_addr - FontBegin.xf_addr, 0);
    }

    App_WrCoCmd_Buffer(g_pHalContext, BITMAP_HANDLE(0));
    Gpu_CoCmd_SetBitmap(g_pHalContext, RAM_DL_SIZE - SS_FRAME_SIZE, SS_ASTC_FORMAT, SCREEN_WIDTH, SCREEN_HEIGHT);
    App_WrCoCmd_Buffer(g_pHalContext, TAG(SS_TAG));
    App_WrCoCmd_Buffer(g_pHalContext, BEGIN(BITMAPS));
    App_WrCoCmd_Buffer(g_pHalContext, VERTEX2F(0, 0));
    App_WrCoCmd_Buffer(g_pHalContext, END());

    App_WrCoCmd_Buffer(g_pHalContext, COLOR_RGB(0, 0, 0));
    App_WrCoCmd_Buffer(g_pHalContext, COLOR_A(ani_color));
    Gpu_CoCmd_Text(g_pHalContext, SCREEN_WIDTH * 0.5, Y_FOOTER, HF_BOTTOM, OPT_CENTERX, s_pleaseTouch);
    App_WrCoCmd_Buffer(g_pHalContext, COLOR_RGB(255, 255, 255));

    ani_color += ani_color_step;
    if (ani_color >= 255) {
        ani_color = 255;
        ani_color_step = -ANI_STEP;
    } else if (ani_color <= 0) {
        ani_color = 0;
        ani_color_step = ANI_STEP * 0.5;
    }

    frame_index++;
    frame_addr += SS_FRAME_SIZE;

    if (frame_index >= SS_FRAME_COUNT) {
        frame_index = 0;
        frame_addr = SS_FLASH_ADDR_FRAME_0;
    }
}

void ss_drawing() {
    load_animation();
}

void ss_process_event() {

    restart_screen_saver();
    uint8_t tag = Gesture_Get(g_pHalContext)->tagReleased;

    if (tag == SS_TAG) {
        switch_page_from_screen_saver();
    }
}
