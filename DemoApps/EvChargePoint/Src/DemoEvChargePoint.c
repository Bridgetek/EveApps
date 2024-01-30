/**
 * @file DemoEvChargePoint.c
 * @brief A demostration of an EV Charge Point station 
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

#include "DemoEvChargePoint.h"
#include "Def.h"
#include "Screen_Saver.h"
#include "Choose_Language.h"
#include "Authenticate.h"
#include "Plugin.h"
#include "Transaction.h"
#include "Charging.h"
#include "Report.h"

#if defined(EVE_FLASH_AVAILABLE)

static EVE_HalContext s_halContext;
EVE_HalContext* s_pHalContext;
void DemoEvChargePoint();

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
	{ "A demostration of an EV Charge Point station ",
		"Support WXGA",
		"EVE3/4",
		"WIN32, FT9XX, IDM2040"
	};

	while (TRUE) {
		WelcomeScreen(s_pHalContext, info);
		DemoEvChargePoint();
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
E_LANG g_language = LANG_EN;
static const E_PAGE PAGE_HOME = PAGE_BEGIN;
static E_PAGE s_currLanguage = PAGE_MAX;
static E_PAGE s_lastLanguage = PAGE_BEGIN;

typedef void(*P_FUNC)();

static P_FUNC fp_draw = 0;
static P_FUNC fp_process = 0;

static uint32_t s_startTime;

static P_FUNC ARR_FUNC_INIT[] = { &ss_init, &cl_init, &ath_init, &pl_init, &tr_init, &ch_init, &rp_init };
static P_FUNC ARR_FUNC_DRAW[] = { &ss_drawing, &cl_drawing, &ath_drawing, &pl_drawing, &tr_drawing, &ch_drawing, &rp_drawing };
static P_FUNC ARR_FUNC_PROCESS[] = { &ss_process_event, &cl_process_event, &ath_process_event, &pl_process_event, &tr_process_event, &ch_process_event, &rp_process_event };

int s_isSwitchPage =-1 ;

void restart_screen_saver() {
    s_startTime = GET_CURR_MILLIS();
}

void switch_page(E_PAGE page) {
    if (page < PAGE_BEGIN && page >= PAGE_MAX)
        return;
    s_currLanguage = page;
    s_isSwitchPage++;

    Display_End(s_pHalContext);

    int index = s_currLanguage - PAGE_BEGIN;
    fp_draw = ARR_FUNC_DRAW[index];
    fp_process = ARR_FUNC_PROCESS[index];
    ARR_FUNC_INIT[index]();
    restart_screen_saver();

    sync_up((uint8_t)page);
}

void switch_next_page() {
    s_currLanguage++;

    if (s_currLanguage >= PAGE_MAX) {
        s_currLanguage = PAGE_BEGIN;
}
    s_lastLanguage = s_currLanguage;
    switch_page(s_currLanguage);
}

void switch_page_from_screen_saver() {
    s_currLanguage = s_lastLanguage;

    if (s_currLanguage == PAGE_BEGIN) {
        s_currLanguage++;
    }
    else if (s_currLanguage >= PAGE_MAX) {
        s_currLanguage = PAGE_BEGIN;
    }
    switch_page(s_currLanguage);
}

void processEvents() {
    uint32_t time_lap = GET_CURR_MILLIS() - s_startTime;

    Gesture_Renew(s_pHalContext);

    fp_process();
    if (time_lap > SS_TIME_OUT) {
        s_startTime = GET_CURR_MILLIS();
        switch_page(PAGE_SCREEN_SAVER);
    }
}

void DemoEvChargePoint() {
    FlashHelper_SwitchFullMode(s_pHalContext);
    Gpu_CoCmd_FlashRead(s_pHalContext, RAM_G, 4096, RAM_DL_SIZE);
    App_WrCoCmd_Buffer(s_pHalContext, VERTEX_FORMAT(FRACTION));
    switch_language();
    switch_page(PAGE_HOME);
    s_startTime = GET_CURR_MILLIS();

    char* s[] = {
        "page_screensaver ",
        "page_language    ",
        "page_authenticate",
        "page_plugin      ",
        "page_transition  ",
        "page_charge      ",
        "page_report      ",
    };

    EVE_Util_clearScreen(s_pHalContext);
    int rc = 0;
    int grc = 0;
	

    uint32_t gt0 = EVE_millis();
    uint32_t t0 = EVE_millis();
    int i = 0;
    while (TRUE) {
        Display_Start(s_pHalContext);
        App_WrCoCmd_Buffer(s_pHalContext, VERTEX_FORMAT(FRACTION));

        fp_draw();
        processEvents();
        Display_End(s_pHalContext);
        rc++;
        grc++;

        if (s_isSwitchPage == 1) {
            s_isSwitchPage = 0;
            uint32_t t1 = EVE_millis();
            float fps = 1000* 1.0 * rc / (t1 - t0);

            printf("%s FPS=%0.2f (%u frames in %u miliseconds)\n", s[i], fps, rc, t1-t0);
            i++;

            if (i == 7) {
                i = 0;
                fps = 1000*1.0 * grc / (t1 - gt0);
                printf("Program FPS=%0.2f(%u frames in %u miliseconds)\n", fps, grc, t1-gt0);
            }

            rc = 0;
            t0 = EVE_millis();
        }
    }

}
#else
#warning Platform is not supported
int main(int argc, char* argv[]) {}
#endif
