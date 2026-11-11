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

/* JAL: Alert interface and output queue handle. Keep for AlertTask startup. */
#include "hh_alert.h"
static QueueHandle_t alertOutputQueue = NULL;
/* JAL: Alert interface and output queue handle. Keep for AlertTask startup. */

void HH_Watchdog_Stub_Task(void *pv);   //DECLARATION: tells the compiler this function exists in another file; must appear B4 the first line using 'HH_Watchdog_Stub_task'

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


void app_start(void){
  
    hh_platform_init();
    hh_println("[HawkHealth] group skeleton starting");
	/* Remove Skeleton task after tasks incorporated*/
	/* Everyone needs to give the Integrator their xTaskCreate line*/
    xTaskCreate(SkeletonTask, "skel", 256, NULL, 2, NULL);

    /* JAL: creates the Alert queue and starts AlertTask. Keep for Alert output. */
    alertOutputQueue = xQueueCreate(1, sizeof(AlertStatus_t));

    if (alertOutputQueue == NULL) {
        hh_println("[ALERT] queue creation failed");
    } else if (xTaskCreate(AlertTask, "alert", 512,
                           alertOutputQueue, 2, NULL) != pdPASS) {
        hh_println("[ALERT] task creation failed");
    }
    /* JAL: creates the Alert queue and starts AlertTask. Keep for Alert output. */

  sensorQueue = xQueueCreate(16, sizeof(SensorSample_t));
  xTaskCreate(SensorTask, "Sensor", 256, NULL, 3, NULL);
  xTaskCreate(HH_Watchdog_Stub_Task, "watchdog", 256, NULL, 2, NULL);   //REGISTRATION: registers the task with the scheduler
  vTaskStartScheduler();//Starts FreeRTOS and never returns, so any xTaskCreate written AFTER it never returns, and those tasks would never exist.
}
