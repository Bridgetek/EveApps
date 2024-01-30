/**
 * @file DemoMediaPlayer.c
 * @brief Manage and play media content on hard disk/SD card
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
#include "DemoMediaPlayer.h"
#if defined(EVE_FLASH_AVAILABLE)

static EVE_HalContext s_halContext;
static EVE_HalContext* s_pHalContext;
void DemoMediaPlayer();

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
	{ "Manage and play media content on hard disk/SD card",
		"Support QVGA, WQVGA, WVGA, WSVGA, WXGA",
		"EVE3/4",
		"WIN32"
	};

	while (TRUE) {
		WelcomeScreen(s_pHalContext, info);
		DemoMediaPlayer();
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
#include "FT_Util.h"
ft_int32_t MultimediaExplorer_Main(Ft_Gpu_Hal_Context_t* pHalContext, GuiManager* CleO, int command, void* _data);
ft_void_t Info(Ft_Gpu_Hal_Context_t* pHalContext);

void DemoMediaPlayer() {
	FlashHelper_SwitchFullMode(s_pHalContext);

    init_guiobs(s_pHalContext);

    while(1) {
        MultimediaExplorer_Main(s_pHalContext, getGuiInstance(), 0, NULL);
    }
}
#else
#warning Platform is not supported
int main(int argc, char* argv[]) {}
#endif