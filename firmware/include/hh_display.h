#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "alert.h"

#define DISPLAY_REFRESH_PERIOD_MS   250u
#define DISPLAY_UART                USART2

bool display_post_alert(const AlertStatus_t *status);
uint32_t display_get_heartbeat(void);
void DisplayTask(void *pv);

#endif
