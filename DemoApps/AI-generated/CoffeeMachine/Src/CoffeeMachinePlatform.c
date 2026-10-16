#include "Common.h"
#include "CoffeeMachinePlatform.h"

bool CoffeeMachine_PlatformInit(EVE_HalContext *host)
{
    Gpu_Init(host);
    return true;
}

bool CoffeeMachine_PlatformPump(void)
{
    return true;
}

void CoffeeMachine_PlatformRelease(EVE_HalContext *host)
{
    Gpu_Release(host);
}
