#ifndef EV_CHARGER_H_
#define EV_CHARGER_H_

#include "Platform.h"

typedef enum EvChargerPage {
    EV_PAGE_IDLE,
    EV_PAGE_CONNECTED,
    EV_PAGE_CHARGING,
    EV_PAGE_SUMMARY,
    EV_PAGE_FAULT,
    EV_PAGE_SETTINGS
} EvChargerPage;

typedef struct EvChargerInputs {
    bool plug_connected;
    bool authorized;
    bool charge_complete;
    bool fault;
    bool fault_retryable;
    bool retry_succeeded;
    uint16_t power_kw;
    uint16_t soc_percent;
    uint32_t energy_deci_kwh;
    uint32_t cost_cents;
    uint32_t elapsed_seconds;
    uint16_t ambient_lux;
    uint8_t hour;
    uint8_t minute;
    bool touch_allowed; /* Set false when the touch controller flags rain/multi-touch noise. */
    const char *fault_code;
    const char *fault_message_en;
    const char *fault_message_zh;
} EvChargerInputs;

/* Replace these weak integration points with charger/BMS/backend I/O. */
void EVCharger_ReadInputs(EvChargerInputs *inputs);
void EVCharger_RequestStop(void);
void EVCharger_RequestCancelAuthorization(void);
void EVCharger_RequestRetry(void);
void EVCharger_Beep(uint16_t milliseconds);
/* Supply one 41-module QR row per call; bit 0 is the leftmost module. */
bool EVCharger_GetQrRow(uint8_t row, uint64_t *module_bits);

#endif
