/*
 * Telemetry.h
 *
 *  Created on: Sep 23, 2026
 *      Author: coredwj
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

//include Alert header for use in telemetry
#include "TheHead.h"


typedef struct {
    uint32_t time_ms; //time to be logged for data
    AlertStatus_t alertStatus; //filter data and its alerts
} TelemetryData_t;

void TelemetryTask(void *pv); // task to take in data, and arrange it into a string for display and log

#endif /* TELEMETRY_H */
