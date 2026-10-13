/* _subsystem_stub_TEMPLATE.c -- copy me into src/<your-subsystem>/ and rename.
 *
 * Integration Lab 1: your stub does NOT implement real logic. It just:
 *   1) receives on your input queue (with a TIMEOUT -- never wait forever),
 *   2) produces a plausible output message,
 *   3) sends it on your output queue,
 *   4) kicks the watchdog each loop.
 * The goal is only to prove the messages flow and the types line up across your seams.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hh_platform.h"
#include "hh_contracts.h"

/* Your task entry. Create it from app_start() with xTaskCreate(). */
void MySubsystemTask(void *pv){
    (void)pv;
    for(;;){
        /* TODO: xQueueReceive(myInQ, &in, pdMS_TO_TICKS(250));  // time out, don't block forever
         *       ... build an output message ...
         *       xQueueSend(myOutQ, &out, 0);
         *       Watchdog_Kick(myHandle); */
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}
