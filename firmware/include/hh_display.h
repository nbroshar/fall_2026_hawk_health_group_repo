#ifndef HH_DISPLAY_H
#define HH_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include "hh_alert.h"

#define DISPLAY_REFRESH_PERIOD_MS   250u

bool display_post_alert(const AlertStatus_t *status);
uint32_t display_get_heartbeat(void);
void DisplayTask(void *pv);

#endif