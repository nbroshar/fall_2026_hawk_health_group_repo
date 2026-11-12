#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "hh_logger.h"
#include "hh_platform.h"


/* Generate fake Telemetry messages for Logger testing */
static TelemetryMessage_t GenerateFakeLog(void)
{
    static uint32_t sample = 0;

    static const char *fakeMessages[] = {
        "Temp: 36.8 Celsius, Alert: NONE\n"
        "Blood Oxygen Level: 98.0%, Alert: NONE\n"
        "Heart Rate: 75 BPM, Alert: NONE\n",

        "Temp: 39.2 Celsius, Alert: HIGH\n"
        "Blood Oxygen Level: 92.5%, Alert: LOW\n"
        "Heart Rate: 115 BPM, Alert: HIGH\n",

        "Temp: ERROR Celsius, Alert: CRITICAL LOW\n"
        "Blood Oxygen Level: 88.0%, Alert: LOW\n"
        "Heart Rate: 45 BPM, Alert: CRITICAL LOW\n"
    };

    TelemetryMessage_t message;

    /* Select a test message at random  */
    const char *source =
        fakeMessages[sample % 3];

    /* Copy message into TelemetryMessage_t */
    strncpy(message.text, source, sizeof(message.text) - 1);
    message.text[sizeof(message.text) - 1] = '\0';

    sample++;

    return message;
}

/* Maximum number of stored telemetry records */
#define MAX_LOG_RECORDS 100

/* Memory storage for telemetry messages */
static TelemetryMessage_t logBuffer[MAX_LOG_RECORDS];

/* Tracks the next position and number of stored records */
static uint32_t logIndex = 0;
static uint32_t logCount = 0;


/* Store telemetry messages in a 100-record circular buffer */
static void StoreLog(const TelemetryMessage_t *message)
{
    /* Copy the incoming message into the next buffer position */
    logBuffer[logIndex] = *message;

    /* Move to the next position, wrapping at 100 */
    logIndex = (logIndex + 1) % MAX_LOG_RECORDS;

    /* Count records until buffer reaches capacity */
    if (logCount < MAX_LOG_RECORDS) {
        logCount++;
    }
}

/* Clear stored logging records */
void LoggerClear(void)
{
    logIndex = 0;
    logCount = 0;
}


/* Logger FreeRTOS task */
void LoggerTask(void *pv)
{
    (void)pv;

    TelemetryMessage_t message;

    for (;;)
    {
        /* Generate Telemetry data */
        message = GenerateFakeLog();

        /* Store the message in memory */
        StoreLog(&message);

        /* Verify the first record was saved in memory */
        if (logCount == 1)
        {
            hh_println("[LOGGER] Verified stored message:");
            hh_println(logBuffer[0].text);
        }

        /* Confirm successful storage through UART */
        hh_print("[LOGGER] Stored record ");
        hh_print_u32(logCount);
        hh_println("/100");

        /* Wait one second before receiving another message */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
