/**
 * @file    hh_filter.h
 * @brief   HawkHealth Filter subsystem: validates raw Sensor samples and
 *          forwards them to Alert as FilteredSample_t.
 *
 * Contract (ISS-1):
 *   - SensorSample_t is defined ONLY in hh_sensor_contract.h (Sensor-owned).
 *     This header includes it and does not redefine it.
 *   - hr_bpm is uint16_t whole BPM in the Sensor contract. Filter converts
 *     it to float when filling FilteredSample_t.fhr_bpm.
 *
 * Data path (ISS-3):
 *   Sensor --sensorQueue (SensorSample_t)--> FilterTask
 *          --filteredQueue (FilteredSample_t)--> Alert
 */
#ifndef HH_FILTER_H
#define HH_FILTER_H

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"

#include "hh_sensor_contract.h"   /* sole definition of SensorSample_t */

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Plausibility limits (inclusive). Outside => sample rejected. ---- */
#define FILTER_TEMP_MIN_C        29.4f
#define FILTER_TEMP_MAX_C        43.3f
#define FILTER_SPO2_MIN_PCT      60.0f
#define FILTER_SPO2_MAX_PCT      100.0f
#define FILTER_HR_MIN_BPM        30.0f
#define FILTER_HR_MAX_BPM        200.0f

/* ---- Task / queue tuning ---- */
#define FILTER_QUEUE_LENGTH      8U     /* depth of filteredQueue            */
#define FILTER_RX_TIMEOUT_MS     500U   /* 2x the 250 ms Sensor period       */
#define FILTER_TX_WAIT_MS        50U    /* bounded wait if Alert falls behind */

/** Filter -> Alert payload. Field names are the agreed Filter-to-Alert
 *  interface; keep them stable. */
typedef struct {
    uint32_t seq;        /* copied from SensorSample_t.seq   */
    uint32_t t_ms;       /* copied from SensorSample_t.t_ms  */
    float    ftemp_c;    /* body temperature, deg C          */
    float    fspo2_pct;  /* SpO2, percent                    */
    float    fhr_bpm;    /* heart rate, BPM (from uint16_t)  */
} FilteredSample_t;

/** Downstream queue, item size sizeof(FilteredSample_t).
 *  Created by Filter_Init(); consumed by Alert. */
extern QueueHandle_t filteredQueue;

/** Create filteredQueue. Call once from app_start() before tasks run.
 *  @return true if the queue exists after the call. */
bool Filter_Init(void);

/** Validate one raw sample and copy it to @p output.
 *  Returns false (and leaves @p output untouched) if either pointer is NULL
 *  or any reading is NaN or outside the inclusive limits above.
 *  On success copies seq/t_ms and the readings, then returns true. */
bool Filter_ProcessSample(const SensorSample_t *input, FilteredSample_t *output);

/** FreeRTOS task: drains sensorQueue, filters, pushes to filteredQueue. */
void FilterTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* HH_FILTER_H */
