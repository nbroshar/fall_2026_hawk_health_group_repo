/*RB and AM Sensor*/
#ifndef SENSOR_CONTRACT_H
#define SENSOR_CONTRACT_H

#include <stdint.h>

typedef struct {
    uint32_t seq;        // sample number: 1, 2, 3, ...
    uint32_t t_ms;       // logical timestamp in milliseconds
    float    temp_c;     // body temperature in degrees C
    float    spo2_pct;   // blood-oxygen saturation, 98.0 = 98% SpO2
    uint16_t hr_bpm;     // heart rate, beats/min
} SensorSample_t;

#endif

/*
 * HawkHealth - SENSOR Subsystem Contract
 *
 * SENSOR responsibilities:
 *   Acquire 3 vital signs from real sensors (FR1)
 *   Update each vital at least 4 Hz (one update every 250 ms or faster) (RT1)
 *   Do not intentionally lose samples under normal load (RT3)
 *
 * Physical sensors:
 *   MAX30102  -> Heart Rate + SpO2
 *   Temperature Sensor -> Body Temperature
 *
 * Data flow:
 *
 *   Physical Sensors
 *         |
 *         v
 *      SENSOR
 *         |
 *         | SensorSample_t
 *         v
 *       FILTER
 *
 * SENSOR acquires the measurements.
 * FILTER determines whether measurements are physiologically plausible.
 * ALERT determines whether valid measurements are NONE / HIGH / CRITICAL.
 */