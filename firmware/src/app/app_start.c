/* app_start.c -- WALKING-SKELETON SEED.  <-- REPLACE this as your team wires real stubs.
 *
 * Right now it proves the whole toolchain end to end: FreeRTOS boots, USART3 comes up,
 * and one task streams a line every 250 ms. That is the green baseline you build on.
 *
 */



#include "FreeRTOS.h"
#include "task.h"
/*Nat was here*/
#include "hh_platform.h"
#include "queue.h"
#include "hh_alert.h"

static QueueHandle_t alertOutputQueue = NULL;

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

    alertOutputQueue = xQueueCreate(1, sizeof(AlertStatus_t));

    if (alertOutputQueue == NULL) {
        hh_println("[ALERT] queue creation failed");
        return;
    }

    if (xTaskCreate(AlertTask, "alert", 512,
                    alertOutputQueue, 2, NULL) != pdPASS) {
        hh_println("[ALERT] task creation failed");
    }
    /* Everyone needs to give the Integrator their xTaskCreate line*/
}
