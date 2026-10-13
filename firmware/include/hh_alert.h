/*JAL Alert*/
#ifndef ALERT_H
#define ALERT_H

#include <stdint.h>

typedef enum {
    ALERT_NONE,
    ALERT_LOW,
    ALERT_HIGH,
    ALERT_CRITICAL_LOW,
    ALERT_CRITICAL_HIGH
} AlertLevel_t;

typedef struct {
    float temp_c;
    AlertLevel_t tempAlert;

    float spo2_pct;
    AlertLevel_t spo2Alert;

    uint16_t hr_bpm;
    AlertLevel_t hrAlert;
} AlertStatus_t;

void AlertTask(void *pv);

#endif