#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "CoffeeMachineController.h"

#if defined(COFFEE_MACHINE_DEMO)
static uint32_t s_brewStarted;
static bool s_brewing;
static bool s_stopRequested;
static bool s_brewNeedsMilk;
#endif

#if defined(__GNUC__)
__attribute__((weak))
#endif
void CoffeeController_ReadStatus(CoffeeStatus *status)
{
    memset(status, 0, sizeof(*status));
    status->boilerCelsius = 93;
    status->waterPercent = 82;
    status->beansPercent = 64;
    status->groundsPercent = 18;
    status->totalSeconds = 30;
#if defined(COFFEE_MACHINE_DEMO)
    status->heartbeatOk = true;
    if (s_brewing) {
        uint32_t seconds = (EVE_millis() - s_brewStarted) / 1000U;
        if (s_stopRequested) {
            status->stopConfirmed = true;
            s_brewing = false;
            s_stopRequested = false;
        } else if (seconds >= status->totalSeconds) {
            status->progress = 100;
            status->step = COFFEE_STEP_DONE;
            status->brewComplete = true;
        } else {
            status->state = COFFEE_CONTROLLER_BUSY;
            status->elapsedSeconds = (uint16_t)seconds;
            status->progress = (uint8_t)(seconds * 100U / status->totalSeconds);
            status->step = seconds < 6U ? COFFEE_STEP_GRINDING :
                (seconds < 22U || !s_brewNeedsMilk ?
                    COFFEE_STEP_BREWING : COFFEE_STEP_MILK);
        }
    }
#else
    status->state = COFFEE_CONTROLLER_ERROR;
#endif
}

#if defined(__GNUC__)
__attribute__((weak))
#endif
void CoffeeController_RequestBrew(const CoffeeBrewRequest *request)
{
    (void)request;
#if defined(COFFEE_MACHINE_DEMO)
    s_brewNeedsMilk = request->milkRatio > 0U;
    s_brewStarted = EVE_millis();
    s_brewing = true;
    s_stopRequested = false;
#endif
}

#if defined(__GNUC__)
__attribute__((weak))
#endif
void CoffeeController_RequestStop(void)
{
#if defined(COFFEE_MACHINE_DEMO)
    s_stopRequested = true;
#endif
}

#if defined(__GNUC__)
__attribute__((weak))
#endif
void CoffeeController_RequestClean(void) { printf("CoffeeMachine: CLEAN request\n"); }

#if defined(__GNUC__)
__attribute__((weak))
#endif
void CoffeeController_RequestStandby(void) { printf("CoffeeMachine: STANDBY request\n"); }

#if defined(__GNUC__)
__attribute__((weak))
#endif
bool CoffeeController_LoadLanguage(CoffeeLanguage *language)
{
#if defined(_WIN32)
    FILE *file = fopen("CoffeeMachineLanguage.cfg", "rb");
    int value;
    if (!file) return false;
    value = fgetc(file);
    fclose(file);
    if (value != '0' && value != '1') return false;
    *language = value == '1' ? COFFEE_LANGUAGE_CHINESE : COFFEE_LANGUAGE_ENGLISH;
    return true;
#else
    (void)language;
    return false;
#endif
}

#if defined(__GNUC__)
__attribute__((weak))
#endif
bool CoffeeController_SaveLanguage(CoffeeLanguage language)
{
#if defined(_WIN32)
    FILE *file = fopen("CoffeeMachineLanguage.cfg", "wb");
    if (!file) return false;
    fputc(language == COFFEE_LANGUAGE_CHINESE ? '1' : '0', file);
    fclose(file);
    return true;
#else
    (void)language;
    return false;
#endif
}
