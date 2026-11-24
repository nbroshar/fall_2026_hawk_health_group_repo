
/* Telemetry subsystem takes in the filtered data with associated alert and packages into a string to send to display to read */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "hh_telemetry.h"
#include "hh_platform.h"

/* Generate random fake data to simulate Alert subsystem output */
static AlertStatus_t GenerateFakeAlert(void)
{
    AlertStatus_t data;

    data.temp_c = 30.0f + (rand() % 151) / 10.0f;

    data.spo2_pct = 60.0f + (rand() % 401) / 10.0f;

    data.hr_bpm = 40 + (rand() % 141);

    /* Random alert level selected*/
    data.tempAlert = (AlertLevel_t)(rand() % 5);
    data.spo2Alert = (AlertLevel_t)(rand() % 5);
    data.hrAlert = (AlertLevel_t)(rand() % 5);

    /* Occasionally generate negative values to test error handling */
    if ((rand() % 20) == 0) {
        data.temp_c = -5.0f;
    }

    if ((rand() % 20) == 0) {
        data.spo2_pct = -10.0f;
    }

    return data;
}

/* Convert alert enum values into readable strings */
static const char *AlertToString(AlertLevel_t alert)
{
    switch (alert) {
        case ALERT_NONE:
            return "NONE";
        case ALERT_LOW:
            return "LOW";
        case ALERT_HIGH:
            return "HIGH";
        case ALERT_CRITICAL_LOW:
            return "CRITICAL LOW";
        case ALERT_CRITICAL_HIGH:
            return "CRITICAL HIGH";
        default:
            return "UNKNOWN";
    }
}



/* Append text to a character buffer */
static void AppendText(char *output, size_t size,
                       size_t *pos, const char *text)
{
    while (*text && *pos < size - 1) {
        output[(*pos)++] = *text++;
    }

    output[*pos] = '\0';
}

/* Append an unsigned integer to a character buffer */
static void AppendNumber(char *output, size_t size,
                         size_t *pos, uint32_t value)
{
    char digits[10];
    int count = 0;

    do {
        digits[count++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    while (count > 0 && *pos < size - 1) {
        output[(*pos)++] = digits[--count];
    }

    output[*pos] = '\0';
}

/* Append a positive float with one decimal place */
static void AppendFloat1(char *output, size_t size,
                         size_t *pos, float value)
{
    uint32_t tenths = (uint32_t)(value * 10.0f + 0.5f);

    AppendNumber(output, size, pos, tenths / 10);
    AppendText(output, size, pos, ".");
    AppendNumber(output, size, pos, tenths % 10);
}

/* Format telemetry measurements into one message */
static void FormatTelemetry(const TelemetryData_t *data,
                            char *output, size_t outputSize)
{
    size_t pos = 0;
    output[0] = '\0';

    /* Temperature */
    AppendText(output, outputSize, &pos, "Temp: ");

    if (data->alertStatus.temp_c < 0.0f) {
        AppendText(output, outputSize, &pos, "ERROR");
    } else {
        AppendFloat1(output, outputSize, &pos,
                     data->alertStatus.temp_c);
    }

    AppendText(output, outputSize, &pos, " Celsius, Alert: ");
    AppendText(output, outputSize, &pos,
               AlertToString(data->alertStatus.tempAlert));
    AppendText(output, outputSize, &pos, "\n");

    /* Blood oxygen */
    AppendText(output, outputSize, &pos, "Blood Oxygen Level: ");

    if (data->alertStatus.spo2_pct < 0.0f) {
        AppendText(output, outputSize, &pos, "ERROR");
    } else {
        AppendFloat1(output, outputSize, &pos,
                     data->alertStatus.spo2_pct);
    }

    AppendText(output, outputSize, &pos, "%, Alert: ");
    AppendText(output, outputSize, &pos,
               AlertToString(data->alertStatus.spo2Alert));
    AppendText(output, outputSize, &pos, "\n");

    /* Heart rate */
    AppendText(output, outputSize, &pos, "Heart Rate: ");
    AppendNumber(output, outputSize, &pos,
                 data->alertStatus.hr_bpm);

    AppendText(output, outputSize, &pos, " BPM, Alert: ");
    AppendText(output, outputSize, &pos,
               AlertToString(data->alertStatus.hrAlert));
    AppendText(output, outputSize, &pos, "\n");
}


/* Print formatted telemetry output through USART3 */
static void PrintTelemetry(const char *message)
{
    hh_println("[TELEMETRY] Display Output:");
    hh_println(message);
}


/* Telemetry FreeRTOS task */
void TelemetryTask(void *pv)
{
    QueueHandle_t outputQueue = (QueueHandle_t)pv;

    TelemetryData_t telemetryData;
    TelemetryMessage_t message;

    for (;;)
    {
        /* Generate simulated Alert subsystem data */
        telemetryData.alertStatus = GenerateFakeAlert();

        /* Timestamp TBA */
        telemetryData.time_ms = 0;

        /* Format data into the queue message */
        FormatTelemetry(&telemetryData,
                        message.text,
                        sizeof(message.text));

        /* Send formatted message to the output queue */
        if (xQueueSend(outputQueue, &message, 0) != pdPASS) {
            hh_println("[TELEMETRY] Output queue full");
        }

        /* Print message to Renode to test */
        PrintTelemetry(message.text);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
