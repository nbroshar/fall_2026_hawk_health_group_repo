/* app_start.c -- WALKING-SKELETON SEED.  <-- REPLACE this as your team wires real stubs.
 *
 * Right now it proves the whole toolchain end to end: FreeRTOS boots, USART3 comes up,
 * and one task streams a line every 250 ms. That is the green baseline you build on.
 *
 */
#include "FreeRTOS.h"
#include "hh_sensor_contract.h"
/*Nat was here*/
#include "hh_platform.h"

#define SENSOR_PERIOD_MS 250   /* 4 Hz (RT1) */

/* Fake readings: temp, spo2, hr */
static const float kTemp[] = { 36.8f, 36.9f, 38.5f, 40.2f, 36.8f };
static const float kSpo2[] = { 98.0f, 97.0f, 93.0f, 86.0f, 98.0f };
static const uint16_t kHr[] = {   72,    75,   115,   150,   300 };  /* 300 = bad data */
#define NUM_SAMPLES 5

QueueHandle_t sensorQueue;

static void SensorTask(void *arg)
{
    (void)arg;
    TickType_t lastWake = xTaskGetTickCount();
    uint32_t seq = 0;

    for (;;) {
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(SENSOR_PERIOD_MS));

        SensorSample_t s;
        uint32_t i = seq % NUM_SAMPLES;

        seq++;
        s.seq      = seq;
        s.t_ms     = seq * SENSOR_PERIOD_MS;
        s.temp_c   = kTemp[i];
        s.spo2_pct = kSpo2[i];
        s.hr_bpm   = kHr[i];

        xQueueSend(sensorQueue, &s, 0);   /* don't block */
    }
}



void app_start(void){
  Serial.begin(115200);
  sensorQueue = xQueueCreate(16, sizeof(SensorSample_t));
  xTaskCreate(SensorTask, "Sensor", 256, NULL, 3, NULL);
  vTaskStartScheduler();
}
