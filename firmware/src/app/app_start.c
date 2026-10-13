/* app_start.c -- WALKING-SKELETON SEED.  <-- REPLACE this as your team wires real stubs.
 *
 * Right now it proves the whole toolchain end to end: FreeRTOS boots, USART3 comes up,
 * and one task streams a line every 250 ms. That is the green baseline you build on.
 *
 * Integration Lab 1: create your queues here, create each owner's stub task, and connect
 * them SENSOR -> FILTER -> ALERT. Include "hh_contracts.h" for the shared types your team
 * agrees on in Part 1. Delete the SkeletonTask once your real stubs stream.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "hh_platform.h"
/* #include "hh_contracts.h"   <-- uncomment once your team has filled it in */

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
    xTaskCreate(SkeletonTask, "skel", 256, NULL, 2, NULL);
}
