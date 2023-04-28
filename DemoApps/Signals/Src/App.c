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
#include "DemoSignals.h"

static EVE_HalContext s_halContext;
static EVE_HalContext *s_pHalContext;

static void mainLoop() {
	// read and store calibration setting
#if !defined(BT8XXEMU_PLATFORM) && GET_CALIBRATION == 1
	Esd_Calibrate(s_pHalContext);
	Calibration_Save(s_pHalContext);
#endif

	EVE_Util_clearScreen(s_pHalContext);

	while (TRUE) {
		DemoSignals(s_pHalContext);
		
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
	mainLoop();

#if defined(MSVC_PLATFORM) || defined(FT900_PLATFORM) || defined(FT93X_PLATFORM)
	return 0;
#endif
}
