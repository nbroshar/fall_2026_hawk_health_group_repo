#include "FreeRTOS.h"
#include "queue.h"      
#include "task.h"
#include "hh_sensor_contract.h"

#define SENSOR_PERIOD_MS 250   /* 4 Hz (RT1) */

QueueHandle_t sensorQueue;

/*
* Fake readings:
*
* Each position across the three arrays represents
* one complete SensorSample_t.
*
* Includes:
*   - normal readings
*   - realistic abnormal readings
*   - bogus sensor readings
*/
 
static const float kTemp[] =
{
    36.8f,     // 0  Normal
    36.9f,     // 1  Normal
    38.5f,     // 2  Fever
    40.2f,     // 3  High fever
 
    36.8f,     // 4  Low SpO2
    36.9f,     // 5  Low heart rate
 
    0.0f,      // 6  BOGUS temperature = 0 C
    250.0f,    // 7  BOGUS temperature too high
 
    36.8f,     // 8  BOGUS SpO2 = 0%
    36.8f,     // 9  BOGUS SpO2 > 100%
 
    36.9f,     // 10 BOGUS heart rate = 0
    36.9f      // 11 BOGUS heart rate too high
};
 
 
static const float kSpo2[] =
{
    98.0f,     // 0  Normal
    97.0f,     // 1  Normal
    93.0f,     // 2  Low
    86.0f,     // 3  Very low
 
    89.0f,     // 4  Low SpO2
    98.0f,     // 5  Normal SpO2
 
    98.0f,     // 6  Used with bogus temperature
    98.0f,     // 7  Used with bogus temperature
 
    0.0f,      // 8  BOGUS SpO2 = 0%
    150.0f,    // 9  BOGUS SpO2 > 100%
 
    98.0f,     // 10 Used with bogus HR
    98.0f      // 11 Used with bogus HR
};
 
 
static const uint16_t kHr[] =
{
    72,        // 0  Normal
    75,        // 1  Normal
    115,       // 2  High HR
    150,       // 3  Very high HR
 
    82,        // 4  Used with low SpO2
    45,        // 5  Bradycardia
 
    72,        // 6  Used with bogus temperature
    75,        // 7  Used with bogus temperature
 
    72,        // 8  Used with bogus SpO2
    75,        // 9  Used with bogus SpO2
 
    0,         // 10 BOGUS heart rate = 0 BPM
    300        // 11 BOGUS heart rate too high
};
 
 
#define NUM_SAMPLES 12

QueueHandle_t sensorQueue;

void SensorTask(void *arg)
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
