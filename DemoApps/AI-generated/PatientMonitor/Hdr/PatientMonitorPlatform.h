#ifndef PATIENT_MONITOR_PLATFORM_H_
#define PATIENT_MONITOR_PLATFORM_H_

#include "Platform.h"

bool PatientMonitor_PlatformInit(EVE_HalContext *phost);
bool PatientMonitor_PlatformPump(void);
/* Emulator-only touch injection, also used by the deterministic smoke test. */
void PatientMonitor_PlatformTouch(int16_t x, int16_t y, bool pressed);
void PatientMonitor_PlatformRelease(EVE_HalContext *phost);

#endif
