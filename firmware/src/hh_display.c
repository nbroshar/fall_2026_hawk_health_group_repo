/* hh_display.c -- DISPLAY subsystem stub (Christian Sivongxay)
*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hh_platform.h"
#include "hh_display.h"


#define DISPLAY_USE_WATCHDOG  0
#if DISPLAY_USE_WATCHDOG
#include "hh_watchdog.h"
#endif

static QueueHandle_t displayQ = NULL;     
static volatile uint32_t heartbeat = 0;

/* ---- fake ALERT data ---- */
static const AlertStatus_t fake_data[] = {
    { 36.8f, ALERT_NONE,          98.0f,  ALERT_NONE,          72,    ALERT_NONE },
    { 35.5f, ALERT_LOW,           93.0f,  ALERT_LOW,           52,    ALERT_LOW },
    { 38.4f, ALERT_HIGH,          98.0f,  ALERT_NONE,          118,   ALERT_HIGH },
    { 34.2f, ALERT_CRITICAL_LOW,  86.0f,  ALERT_CRITICAL_LOW,  38,    ALERT_CRITICAL_LOW },
    { 39.6f, ALERT_CRITICAL_HIGH, 97.0f,  ALERT_NONE,          145,   ALERT_CRITICAL_HIGH },
    /* bogus: sensor lead fell off, everything reads zero */
    { 0.0f,  ALERT_NONE,          0.0f,   ALERT_NONE,          0,     ALERT_NONE },
    /* bogus: impossible values */
    { 85.0f, ALERT_NONE,          120.0f, ALERT_NONE,          400,   ALERT_NONE },
    /* bogus: negative temp, maxed-out heart rate */
    { -12.5f, ALERT_NONE,         99.0f,  ALERT_NONE,          65535, ALERT_NONE },
};
#define FAKE_COUNT (sizeof(fake_data) / sizeof(fake_data[0]))

/* ---- services other subsystems call ---- */


bool display_post_alert(const AlertStatus_t *status)
{
    if (status == NULL || displayQ == NULL) return false;
    return xQueueOverwrite(displayQ, status) == pdPASS;
}


uint32_t display_get_heartbeat(void)
{
    return heartbeat;
}

/* ---- helpers ---- */

static const char *lvl(AlertLevel_t l)
{
    switch (l) {
        case ALERT_LOW:           return "LOW";
        case ALERT_HIGH:          return "HIGH";
        case ALERT_CRITICAL_LOW:  return "CRITICAL_LOW";
        case ALERT_CRITICAL_HIGH: return "CRITICAL_HIGH";
        default:                  return "NONE";
    }
}

static void print_float(float f)
{
    if (f < 0.0f) { hh_print("-"); f = -f; }
    hh_print_float1(f);
}

/* Values no real patient could have */
static bool temp_ok(float t)  { return t >= 25.0f && t <= 45.0f; }
static bool spo2_ok(float s)  { return s > 0.0f  && s <= 100.0f; }
static bool hr_ok(uint16_t h) { return h > 0     && h <= 250; }

static void show(const AlertStatus_t *s)
{
    bool bad = !temp_ok(s->temp_c) || !spo2_ok(s->spo2_pct) || !hr_ok(s->hr_bpm);

    hh_print("[DISPLAY STUB] temp=");
    print_float(s->temp_c);
    hh_print(" C ");
    hh_print(temp_ok(s->temp_c) ? lvl(s->tempAlert) : "BAD");

    hh_print(" | spo2=");
    print_float(s->spo2_pct);
    hh_print("% ");
    hh_print(spo2_ok(s->spo2_pct) ? lvl(s->spo2Alert) : "BAD");

    hh_print(" | hr=");
    hh_print_u32(s->hr_bpm);
    hh_print(" bpm ");
    hh_print(hr_ok(s->hr_bpm) ? lvl(s->hrAlert) : "BAD");

    hh_println(bad ? " | Outlier" : "");
}

/* ---- the task ---- */

void DisplayTask(void *pv)
{
    (void)pv;
    uint32_t i = 0;
    AlertStatus_t in;

    displayQ = xQueueCreate(1, sizeof(AlertStatus_t));
    if (displayQ == NULL) {
        hh_println("[DISPLAY STUB] queue create failed");
        vTaskDelete(NULL);
    }

#if DISPLAY_USE_WATCHDOG
    Watchdog_ChannelConfig_t cfg = { "DISPLAY", 500u, NULL };
    WatchdogHandle_t wd = Watchdog_Register(&cfg);
#endif

    for (;;) {
        display_post_alert(&fake_data[i]);
        i = (i + 1) % FAKE_COUNT;

        if (xQueueReceive(displayQ, &in, pdMS_TO_TICKS(DISPLAY_REFRESH_PERIOD_MS)) == pdPASS) {
            show(&in);
        } else {
            hh_println("[DISPLAY STUB] no data (timeout)");
        }

        heartbeat++;
#if DISPLAY_USE_WATCHDOG
        Watchdog_Kick(wd);
#endif
        vTaskDelay(pdMS_TO_TICKS(DISPLAY_REFRESH_PERIOD_MS));
    }
}