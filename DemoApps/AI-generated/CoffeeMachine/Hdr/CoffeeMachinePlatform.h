#ifndef COFFEE_MACHINE_PLATFORM_H_
#define COFFEE_MACHINE_PLATFORM_H_

#include <stdbool.h>
#include <stdint.h>
#include "Platform.h"

bool CoffeeMachine_PlatformInit(EVE_HalContext *host);
bool CoffeeMachine_PlatformPump(void);
void CoffeeMachine_PlatformRelease(EVE_HalContext *host);

#endif
