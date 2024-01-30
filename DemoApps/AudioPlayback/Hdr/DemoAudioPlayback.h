#ifndef DEMOAUDIOPLAYBACK_H_
#define DEMOAUDIOPLAYBACK_H_

#include "platform.h"

/** Path to UI assets Folder */
#if defined(_WIN32) 
/** location on PC */
#define TEST_DIR "..\\..\\..\\Test\\"

#elif defined(EMBEDDED_PLATFORM) 
/** location on sdcard */
#define TEST_DIR "/EveApps/DemoAudioPlayback/Test"
#else
#define TEST_DIR "/"
#endif

#define GET_CALIBRATION                     1

#endif /* DEMOAUDIOPLAYBACK_H_ */
