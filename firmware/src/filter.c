/**
 * @file    hh_filter.c
 * @brief   HawkHealth Filter subsystem: validation + pass-through stub.
 *
 * Plausibility filter only. A clinically abnormal but physically plausible
 * reading (e.g. 40.2 C / 97 % / 85 BPM) PASSES so Alert can judge it.
 * A bogus reading (e.g. 0 C) is rejected.
 */
#include "hh_filter.h"

#include <math.h>   /* isnan */
#include <stddef.h> /* NULL  */

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

QueueHandle_t filteredQueue = NULL;

/* Diagnostics, readable from a debugger / Renode monitor. */
static volatile uint32_t s_rejected_count = 0U;
static volatile uint32_t s_dropped_count  = 0U;

static bool in_range_f(float v, float lo, float hi)
{
    return (!isnan(v)) && (v >= lo) && (v <= hi);
}

bool Filter_Init(void)
{
    if (filteredQueue == NULL) {
        filteredQueue = xQueueCreate(FILTER_QUEUE_LENGTH,
                                     (UBaseType_t)sizeof(FilteredSample_t));
    }
    return (filteredQueue != NULL);
}

bool Filter_ProcessSample(const SensorSample_t *input, FilteredSample_t *output)
{
    if ((input == NULL) || (output == NULL)) {
        return false;
    }

    /* Convert to float first. Then the same check covers float or integer
     * fields. hr_bpm is uint16_t whole BPM, so this conversion is exact. */
    const float temp_c   = (float)input->temp_c;
    const float spo2_pct = (float)input->spo2_pct;
    const float hr_bpm   = (float)input->hr_bpm;

    /* Validate everything before touching *output. */
    if (!in_range_f(temp_c,   FILTER_TEMP_MIN_C,   FILTER_TEMP_MAX_C)   ||
        !in_range_f(spo2_pct, FILTER_SPO2_MIN_PCT, FILTER_SPO2_MAX_PCT) ||
        !in_range_f(hr_bpm,   FILTER_HR_MIN_BPM,   FILTER_HR_MAX_BPM)) {
        return false;
    }

    output->seq       = input->seq;
    output->t_ms      = input->t_ms;
    output->ftemp_c   = temp_c;
    output->fspo2_pct = spo2_pct;
    output->fhr_bpm   = hr_bpm;
    return true;
}

/* ISS-4: on main, hh_watchdog.h is a header with no .c in the build, so any
 * call would fail to link. This is the single kick point. It runs on every
 * loop pass, including a receive timeout. Fill it in once the watchdog lands
 * and its Register/Kick signatures are confirmed. */
static void filter_watchdog_kick(void)
{
}

void FilterTask(void *pvParameters)
{
    (void)pvParameters;

    const TickType_t rx_timeout = pdMS_TO_TICKS(FILTER_RX_TIMEOUT_MS);
    const TickType_t tx_wait    = pdMS_TO_TICKS(FILTER_TX_WAIT_MS);
    SensorSample_t   raw;
    FilteredSample_t out;

    for (;;) {
        if (sensorQueue == NULL) {
            vTaskDelay(rx_timeout);          /* never spin; never block forever */
        } else if (xQueueReceive(sensorQueue, &raw, rx_timeout) == pdPASS) {
            if (!Filter_ProcessSample(&raw, &out)) {
                s_rejected_count++;
            } else if ((filteredQueue == NULL) ||
                       (xQueueSend(filteredQueue, &out, tx_wait) != pdPASS)) {
                s_dropped_count++;           /* Alert behind / queue missing */
            } else {
                /* forwarded */
            }
        } else {
            /* receive timeout: fall through to the kick */
        }

        filter_watchdog_kick();
    }
}
