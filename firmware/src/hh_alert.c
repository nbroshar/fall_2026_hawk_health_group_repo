#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hh_alert.h"
#include "hh_platform.h"

static const AlertStatus_t samples[] = {
    {36.8f, ALERT_NONE, 98.0f, ALERT_NONE, 75, ALERT_NONE},
    {35.5f, ALERT_LOW, 93.0f, ALERT_LOW, 55, ALERT_LOW},
    {38.5f, ALERT_HIGH, 98.0f, ALERT_NONE, 115, ALERT_HIGH},
    {33.0f, ALERT_CRITICAL_LOW, 85.0f, ALERT_CRITICAL_LOW, 35, ALERT_CRITICAL_LOW},
    {41.0f, ALERT_CRITICAL_HIGH, 98.0f, ALERT_NONE, 180, ALERT_CRITICAL_HIGH},
    {109.3f, ALERT_CRITICAL_HIGH, 56.1f, ALERT_CRITICAL_LOW, 35, ALERT_CRITICAL_LOW},
    {36.8f, ALERT_NONE, 110.0f, ALERT_CRITICAL_HIGH, 200, ALERT_CRITICAL_HIGH}
};

static const char *level_name(AlertLevel_t level)
{
    switch (level) {
        case ALERT_NONE:          return "NONE";
        case ALERT_LOW:           return "LOW";
        case ALERT_HIGH:          return "HIGH";
        case ALERT_CRITICAL_LOW:  return "CRITICAL_LOW";
        case ALERT_CRITICAL_HIGH: return "CRITICAL_HIGH";
        default:                  return "UNKNOWN";
    }
}

static void print_decimal(float value)
{
    uint32_t tenths = (uint32_t)(value * 10.0f + 0.5f);

    hh_print_u32(tenths / 10);
    hh_print(".");
    hh_print_u32(tenths % 10);
}

void AlertTask(void *pv)
{
    QueueHandle_t output = (QueueHandle_t)pv;
    uint32_t index = 0;
    const uint32_t count = sizeof(samples) / sizeof(samples[0]);

    if (output == NULL) {
        hh_println("[ALERT] missing output queue");
        vTaskDelete(NULL);
        return;
    }

    for (;;) {
        AlertStatus_t status = samples[index];

        if (xQueueOverwrite(output, &status) == pdPASS) {
            hh_print("[ALERT STUB] temp=");
            print_decimal(status.temp_c);
            hh_print(" C ");
            hh_print(level_name(status.tempAlert));

            hh_print(" | spo2=");
            print_decimal(status.spo2_pct);
            hh_print("% ");
            hh_print(level_name(status.spo2Alert));

            hh_print(" | hr=");
            hh_print_u32(status.hr_bpm);
            hh_print(" bpm ");
            hh_print(level_name(status.hrAlert));

            if (index >= 5) {
                hh_print(" | Clear Outlier");
            }

            hh_println("");
        } else {
            hh_println("[ALERT] queue write failed");
        }

        index = (index + 1) % count;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
