/**
 * @file Report.c
 * @brief Report page
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

#include "Report.h"
#include "Def.h"

#include "DemoEvChargePoint.h"
#include "Transaction.h"
#include "common.h"


// Global variables
extern Gpu_Hal_Context_t* s_pHalContext;
extern E_LANG g_language;

extern const char * s_report;
extern const char * s_exit;
extern const char * s_chargingTime;
extern const char * s_battery;
extern const char * s_batteryHealth;
extern const char * s_totalCost;
extern const char * s_minutes_report;
extern const char * s_batteryHealth;
extern const char * s_healthy;

static bool_t start_ani_out;
int16_t y_car;
int16_t rp_alpha;

void rp_init() {
	start_ani_out = FALSE;
	y_car = 100;
	rp_alpha = 255;
}

void rp_drawing() {

    int x = 0, y = 0;

    Gpu_CoCmd_SetFont2(s_pHalContext, HF_TITLE, FontTitle.xf_addr - FontBegin.xf_addr, 0);
    Gpu_CoCmd_SetFont2(s_pHalContext, HF_SMALL, FontSmall.xf_addr - FontBegin.xf_addr, 0);
    Gpu_CoCmd_SetFont2(s_pHalContext, HF_NUMBER, FontNumber.xf_addr - FontBegin.xf_addr, 0);



    // Draw Car
    draw_image(s_pHalContext, TR_CAR, ADDR(TR_CAR, TR_BEGIN), H_CENTER(TR_CAR.width), y_car);

    if (start_ani_out) {
    	y_car -= 10;
    	rp_alpha -= 3;

    	if (rp_alpha < 0)
    		rp_alpha = 0;
    }

    App_WrCoCmd_Buffer(s_pHalContext, COLOR_A(rp_alpha));

    // Draw QR Code
    draw_image(s_pHalContext, TR_QR_CODE, ADDR(TR_QR_CODE, TR_BEGIN), SCREEN_WIDTH - TR_QR_CODE.width - 100,
    		100 + (TR_CAR.height - TR_QR_CODE.height) * 0.5);

    // Draw Text: header
    App_WrCoCmd_Buffer(s_pHalContext, COLOR_RGB(0, 0, 0));
    Gpu_CoCmd_Text(s_pHalContext, X_HEADER, Y_HEADER, HF_TITLE, 0, s_report);
    App_WrCoCmd_Buffer(s_pHalContext, COLOR_RGB(255, 255, 255));

    // Draw Exit
    x = H_CENTER(TR_EXIT_BUTTON.width);
    y = 700;
    
    App_WrCoCmd_Buffer(s_pHalContext, COLOR_RGB(0, 85, 255));
    Gpu_CoCmd_Text(s_pHalContext, x + TR_EXIT_BUTTON.width * 0.5, y + TR_EXIT_BUTTON.height * 0.5, HF_TITLE, OPT_CENTER, s_exit);
    draw_image_with_tag(s_pHalContext, TR_EXIT_BUTTON, ADDR(TR_EXIT_BUTTON, TR_BEGIN), x, y - 3, RE_TAG_EXIT_BUTTON);

    // Draw Report
    x = 380; y = 500;
    int16_t HF_LEGEND = 30;

    if (g_language == LANG_CN)
        HF_LEGEND = HF_SMALL;

    App_WrCoCmd_Buffer(s_pHalContext, COLOR_GREY);
    Gpu_CoCmd_Text(s_pHalContext, x, y - 3, HF_LEGEND, OPT_RIGHTX, s_battery);
    Gpu_CoCmd_Text(s_pHalContext, SCREEN_WIDTH - x, y - 3, HF_LEGEND, OPT_RIGHTX, s_batteryHealth);
    Gpu_CoCmd_Text(s_pHalContext, x, y + 67, HF_LEGEND, OPT_RIGHTX, s_chargingTime);
    Gpu_CoCmd_Text(s_pHalContext, SCREEN_WIDTH - x, y + 67, HF_LEGEND, OPT_RIGHTX, s_totalCost);

    App_WrCoCmd_Buffer(s_pHalContext, COLOR_RGB(0, 85, 255));
    Gpu_CoCmd_Text(s_pHalContext, x + 10, y - 12, HF_NUMBER, 0, "100");
    App_WrCoCmd_Buffer(s_pHalContext, COLOR_RGB(0, 0, 0));
    Gpu_CoCmd_Text(s_pHalContext, x + 110, y - 12, 31, 0, "%");

    Gpu_CoCmd_Number(s_pHalContext, x + 10, y + 57, HF_NUMBER, 0, 120);
    Gpu_CoCmd_Text(s_pHalContext, x + 110, y + 67, HF_LEGEND, 0, s_minutes_report);

    App_WrCoCmd_Buffer(s_pHalContext, COLOR_RGB(0, 85, 255));
    Gpu_CoCmd_Text(s_pHalContext, SCREEN_WIDTH - x + 15, y - 9, HF_TITLE, 0, s_healthy);

    App_WrCoCmd_Buffer(s_pHalContext, COLOR_RGB(0, 0, 0));
    Gpu_CoCmd_Text(s_pHalContext, SCREEN_WIDTH - x + 15, y + 55, 31, 0, "$");

    Gpu_CoCmd_Text(s_pHalContext, SCREEN_WIDTH - x + 45, y + 57, HF_NUMBER, 0, "32");
}

void rp_process_event() {
    uint8_t tag = Gesture_Get(s_pHalContext)->tagReleased;

    if (tag == RE_TAG_EXIT_BUTTON) {
    	start_ani_out = TRUE;
    }

    if (y_car <= -TR_CAR.height) {
        switch_next_page();
    }
}
