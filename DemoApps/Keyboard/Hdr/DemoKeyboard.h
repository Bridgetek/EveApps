#ifndef DEMOKEYBOARD_H_
#define DEMOKEYBOARD_H_

#include "platform.h"

/** Path to UI assets Folder */
#if defined(_WIN32) 
/** location on PC */
#define TEST_DIR "..\\..\\..\\Test\\"

#elif defined(EMBEDDED_PLATFORM) 
/** location on sdcard */
#define TEST_DIR "/EveApps/DemoKeyboard/Test"
#else
#define TEST_DIR "/"
#endif

#define GET_CALIBRATION                     1

#endif /* DEMOKEYBOARD_H_ */
