/*
 * Logger.h
 *
 *  Created on: Oct 13, 2026
 *      Author: coredwj
 */

#ifndef LOGGER_H
#define LOGGER_H

#include "telemetry.h" //include telemetry so it can receive data from telemetry.c

void LoggerTask(void *pv);// store data in log
void LoggerClear(void); //clear log function

#endif /* LOGGER_H */
