/**
 * @file App.c
 * @brief Application startup
 *
 * @author Bridgetek
 *
 * @date 2019
 */

#include "Common.h"
#include "App.h"
#include "DemoMediaPlayer.h"
#if defined(EVE_FLASH_AVAILABLE)

static EVE_HalContext s_halContext;
static EVE_HalContext *s_pHalContext;

static void mainLoop() {
	EVE_Util_clearScreen(s_pHalContext);

	while (TRUE) {
		DemoMediaPlayer(s_pHalContext);
		
		EVE_Util_clearScreen(s_pHalContext);

		EVE_Hal_close(s_pHalContext);
		EVE_Hal_release();

		/* Init HW Hal for next loop*/
		Gpu_Init(s_pHalContext);
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
		Calibration_Restore(s_pHalContext);
#endif
	}
}

#if defined(ARDUINO_PLATFORM)
void EVE_emuMain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	s_pHalContext = &s_halContext;
	Gpu_Init(s_pHalContext);

	if (s_pHalContext->Width > 1023)
		Setup_Precision(3); // -2048 to 2047. Note: Use VP with vertex

	// read and store calibration setting
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
	Esd_Calibrate(s_pHalContext);
	Calibration_Save(s_pHalContext);
#endif

	Flash_Init(s_pHalContext, TEST_DIR "/Flash/BT81X_Flash.bin", "BT81X_Flash.bin");

	mainLoop();

#if defined(MSVC_PLATFORM) || defined(FT900_PLATFORM) || defined(FT93X_PLATFORM)
	return 0;
#endif
}
#else
#if defined(ARDUINO_PLATFORM)
void EVE_emuMain(int argc, char *argv[])
#else
int main(int argc, char* argv[]) {}
#endif
#endif

