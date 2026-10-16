#ifndef COFFEE_MACHINE_CONTROLLER_H_
#define COFFEE_MACHINE_CONTROLLER_H_

#include <stdbool.h>
#include <stdint.h>
#include "CoffeeMachine.h"

typedef enum CoffeeControllerState {
    COFFEE_CONTROLLER_READY = 0,
    COFFEE_CONTROLLER_BUSY,
    COFFEE_CONTROLLER_ERROR
} CoffeeControllerState;

typedef enum CoffeeBrewStep {
    COFFEE_STEP_GRINDING = 0,
    COFFEE_STEP_BREWING,
    COFFEE_STEP_MILK,
    COFFEE_STEP_DONE
} CoffeeBrewStep;

typedef struct CoffeeStatus {
    CoffeeControllerState state;
    CoffeeBrewStep step;
    uint8_t progress;
    uint16_t elapsedSeconds;
    uint16_t totalSeconds;
    int16_t boilerCelsius;
    uint8_t waterPercent;
    uint8_t beansPercent;
    uint8_t groundsPercent;
    bool heartbeatOk;
    bool brewComplete;
    bool stopConfirmed;
} CoffeeStatus;

typedef struct CoffeeBrewRequest {
    CoffeeDrinkId drink;
    uint16_t volumeMl;
    uint8_t strength;
    uint8_t temperature;
    uint8_t milkRatio;
} CoffeeBrewRequest;

void CoffeeController_ReadStatus(CoffeeStatus *status);
void CoffeeController_RequestBrew(const CoffeeBrewRequest *request);
void CoffeeController_RequestStop(void);
void CoffeeController_RequestClean(void);
void CoffeeController_RequestStandby(void);
bool CoffeeController_LoadLanguage(CoffeeLanguage *language);
bool CoffeeController_SaveLanguage(CoffeeLanguage language);

#endif
