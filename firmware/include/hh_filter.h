/*VY Filter*/
#ifndef HAWKHEALTH_FILTER_H
#define HAWKHEALTH_FILTER_H

#include <stdint.h>
#include <stdbool.h>


/* =========================================================
 * SENSOR -> FILTER
 *
 * Data produced by the Sensor subsystem and consumed
 * by the Filter subsystem.
 * (STILL WAITING FOR SENSOR TO GIVE ME THEIR HEADER FILE)
 * ========================================================= */

typedef struct
{
    uint32_t seq;          // Sample sequence number
    uint32_t t_ms;         // Time sample was taken (ms)

    float temp_c;          // Body temperature (degrees C)
    float spo2_pct;        // Blood oxygen (%)
    float hr_bpm;          // Heart rate (BPM)

} SensorSample_t;


/* =========================================================
 * FILTER -> ALERT
 *
 * Data produced by the Filter subsystem and consumed
 * by the Alert subsystem.
 * ========================================================= */

typedef struct
{
    uint32_t seq;          // Original sample sequence number
    uint32_t t_ms;         // Original sample timestamp

    float ftemp_c;         // Validated temperature (degrees C)
    float fspo2_pct;       // Validated SpO2 (%)
    float fhr_bpm;         // Validated heart rate (BPM)

} FilteredSample_t;


/* =========================================================
 * FILTER API
 *
 * Public services provided by the Filter subsystem.
 * ========================================================= */

/*
 * Validate a sensor sample and produce a filtered sample.
 *
 * Returns:
 *   true  - sample is valid and output was produced
 *   false - sample was rejected
 */
bool Filter_ProcessSample(const SensorSample_t *input,
                          FilteredSample_t *output);


#endif /* HAWKHEALTH_FILTER_H */