/* app_start.c -- WALKING-SKELETON SEED.  <-- REPLACE this as your team wires real stubs.
 *
 * Right now it proves the whole toolchain end to end: FreeRTOS boots, USART3 comes up,
 * and one task streams a line every 250 ms. That is the green baseline you build on.
 *
 */
#include "FreeRTOS.h"
#include "hh_sensor_contract.h"
#include "task.h"
#include "queue.h"
#include "hh_platform.h"
#include "hh_display.h"
#include "hh_logger.h"

/* JAL: Alert interface and output queue handle. Keep for AlertTask startup. */
#include "hh_alert.h"
static QueueHandle_t alertOutputQueue = NULL;
/* JAL: Alert interface and output queue handle. Keep for AlertTask startup. */

void HH_Watchdog_Stub_Task(void *pv);   //DECLARATION: tells the compiler this function exists in another file; must appear B4 the first line using 'HH_Watchdog_Stub_task'

#include "hh_telemetry.h"


static void SkeletonTask(void *pv){
    (void)pv;
    uint32_t seq = 0;
    for(;;){
        hh_print("[SKELETON] seq=");
        hh_print_u32(seq++);
        hh_println("");
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}


/* Temporary Display task to test Telemetry output queue */
static void TestDisplayTask(void *pv)
{
    QueueHandle_t queue = (QueueHandle_t)pv;
    TelemetryMessage_t received;

    for (;;)
    {
        if (xQueueReceive(queue, &received, portMAX_DELAY) == pdPASS)
        {
            hh_println("[DISPLAY TEST] Received from Telemetry:");
            hh_println(received.text);
        }
    }
}


void app_start(void){
  
    hh_platform_init();
    hh_println("[HawkHealth] group skeleton starting");
	/* Remove Skeleton task after tasks incorporated*/
	/* Everyone needs to give the Integrator their xTaskCreate line*/
    xTaskCreate(SkeletonTask, "skel", 256, NULL, 2, NULL);

    /* JAL: creates the Alert queue and starts AlertTask. Keep for Alert output. */
    alertOutputQueue = xQueueCreate(1, sizeof(AlertStatus_t));

    static QueueHandle_t telemetryOutputQueue = NULL;

    if (alertOutputQueue == NULL) {
        hh_println("[ALERT] queue creation failed");
    } else if (xTaskCreate(AlertTask, "alert", 512,
                           alertOutputQueue, 2, NULL) != pdPASS) {
        hh_println("[ALERT] task creation failed");
    }
    /* JAL: creates the Alert queue and starts AlertTask. Keep for Alert output. */
    /* Create Telemetry output queue */
    telemetryOutputQueue = xQueueCreate(4, sizeof(TelemetryMessage_t));

    if (telemetryOutputQueue == NULL) {
        hh_println("[TELEMETRY] Queue creation failed");
    }
    else if (xTaskCreate(TelemetryTask, "Telemetry", 512,
                         telemetryOutputQueue, 2, NULL) != pdPASS) {
        hh_println("[TELEMETRY] Task creation failed");
    }
    /* Temporary receiver to verify Telemetry queue */
    if (telemetryOutputQueue != NULL) {
        if (xTaskCreate(TestDisplayTask, "DisplayTest", 768,
                        telemetryOutputQueue, 2, NULL) != pdPASS) {
            hh_println("[DISPLAY TEST] Task creation failed");
        }
    }

    /* Start Logger subsystem */
    if (xTaskCreate(LoggerTask, "Logger", 512,
                    NULL, 2, NULL) != pdPASS) {
        hh_println("[LOGGER] Task creation failed");
    }

  sensorQueue = xQueueCreate(16, sizeof(SensorSample_t));
  xTaskCreate(SensorTask, "Sensor", 256, NULL, 3, NULL);
  xTaskCreate(HH_Watchdog_Stub_Task, "watchdog", 256, NULL, 2, NULL);   //REGISTRATION: registers the task with the scheduler
  xTaskCreate(DisplayTask, "Display", 256, NULL, 1, NULL);
 vTaskStartScheduler();//Starts FreeRTOS and never returns, so any xTaskCreate written AFTER it never returns, and those tasks would never exist.
}
