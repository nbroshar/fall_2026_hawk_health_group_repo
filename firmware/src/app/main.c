/* main.c -- HawkHealth group repo entry point (SUPPORT; leave as-is).
 * Calls app_start() (which your team owns), then hands control to the scheduler. */
#include "FreeRTOS.h"
#include "task.h"
extern void app_start(void);
int main(void){
    app_start();
    vTaskStartScheduler();
    for(;;){}
    return 0;
}
