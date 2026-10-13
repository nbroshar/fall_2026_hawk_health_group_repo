/*watchdog.h
owner: pimentel, daniela (HEALTH)
requirements:
    FR4: (functional requirement 4): detect any subsystem stopped responding and drive a safe fault action
	RT5: (real-time constraint 5): a hung/silent subsystem is detected AND acted on w/in <= 1s
	
	Goal: watchdog monitors registered tasks and takes action if one stops reporting in on time
	Watches: for "is your loop still running?", not checking for data quality; that's responsibility of FILTER
	        8 channel (loops) to watch:
			Sensor (3)
			Filter (1)
			Alert (1)
			Telemetry (1)
			Display (1)
			Command (1)
			
	"Kick" = tell watchdog "I'm still alive."
	
* HOW TASKS THAT WAIT ON A QUEUE SHOULD KICK (FILTER, ALERT, COMMAND):
    Wait on the queue with a timeout, and kick every time the wait ends,
    even if no message arrived. Kicking only when you produce output would
   reset a healthy, idle board.
   (FreeRTOS: xQueueReceive. CubeMX's wrapper: osMessageQueueGet.)

 * NEVER WAIT FOREVER (use cases 06 and 10):
    A task that waits forever stops kicking, and the board resets.
    - Missing hardware: time out, mark the device absent, keep looping.
    - Slow output (full serial buffer): don't wait for space. Drop the new
      message and count it, or overwrite the oldest.
 
 * SAFE TO CALL from any task. Kick is quick and never blocks.
    Never call from an ISR (Interrupt Service Routine, code that interrupts tasks).
 
 * PRIORITY: the supervisor task runs above the tasks it watches, so busy
    tasks can't starve it. TODO(team): confirm the numbers with ALERT's owner.
    The module refreshes the chip's IWDG (Independent Watchdog, a hardware reset
    timer) only while every channel is healthy. Callers never touch it.
*/


	
#ifndef WATCHDOG_H
#define WATCHDOG_H
	
#include <stdint.h>
#include <stdbool.h>
	
/*-------TIMING CONTRACT (from RT5)--------*/
//RT5: hung subsystem must be detected and acted on w/in this time
#define WATCHDOG_DEADLINE_MS (1000u)
	
	
//How often the supervisor checks every channel. Worst-case detection = timeout_ms + this period
#define WATCHDOG_CHECK_PERIOD_MS (100u)
	
//Largest timeout_ms accepted: timeout + check period must still fit inside the deadline (1000 - 100 = 900 ms)
#define WATCHDOG_MAX_TIMEOUT_MS (WATCHDOG_DEADLINE_MS - WATCHDOG_CHECK_PERIOD_MS)
	
//Smallest timeout_ms accepted
#define WATCHDOG_MIN_TIMEOUT_MS (WATCHDOG_CHECK_PERIOD_MS)



	
//-------Types-------//
	
//Returned by Watchdog_Register(), to be passed to every later call
typedef uint8_t WatchdogHandle_t;

//Returned by Watchdog_Register() when registration fails (no free slot, config == NULL, or timeout_ms outside [MIN,MAX]
#define WATCHDOG_INVALID_HANDLE ((WatchdogHandle_t) 0xFFu)
	
	
//How many channels can run at once; 2 spare slots for wiggle room
//@ Ahmed/John: are the 3 sensors 3 tasks or 1 task?
//@Jose: Does Config have any tasks?
#define WATCHDOG_MAX_CHANNELS (10u)


//Callback run when a channel times out. Runs in WATCHDOG SUPERVISOR task.
typedef void (*Watchdog_TimeoutCb_t)(WatchdogHandle_t handle, const char *name);



/*Configuration for one channel (one task = one channel).
Watchdog copies what it needs during Register, so struct may live on caller's stack. */
typedef struct {
    const char *name;                  // which subsystem this is; used for logging/debug
    uint32_t timeout_ms;               // max time between kicks before a fault;
                                       // must be within [WATCHDOG_MIN_TIMEOUT_MS, WATCHDOG_MAX_TIMEOUT_MS]                       
    Watchdog_TimeoutCb_t  on_timeout;  // optional per-channel callback; NULL = use the default system action        
} Watchdog_ChannelConfig_t;

/** Health of one channel, as reported by Watchdog_GetChannelStatus(). */
typedef enum {
    WATCHDOG_CH_UNUSED = 0,   /* not registered (or deregistered / bad handle) */
    WATCHDOG_CH_OK,           /* kicking on time                               */
    WATCHDOG_CH_TIMED_OUT     /* missed its timeout_ms                         */
} Watchdog_ChannelStatus_t;

/* Why the board last reset (use case 09: "note that the last stop was a reset").
 */
typedef enum {
    WATCHDOG_RESET_UNKNOWN = 0,
    WATCHDOG_RESET_POWER_ON,   /* POR (Power-On Reset)                 */
    WATCHDOG_RESET_PIN,        /* NRST pin, i.e. the reset button      */
    WATCHDOG_RESET_BROWNOUT,   /* BOR (Brown-Out Reset), supply dipped */
    WATCHDOG_RESET_SOFTWARE,   /* firmware requested reset             */
    WATCHDOG_RESET_IWDG,       /* Independent Watchdog fired           */
    WATCHDOG_RESET_WWDG,       /* WWDG (Window Watchdog) fired         */
    WATCHDOG_RESET_LOW_POWER   /* low-power mode entry reset           */
} WatchdogResetReason_t;



//-------Functions-------//


/* 
@brief   One-time module setup. Calls once before any subsystem registers. Latches reset reason and creates the supervisor task.
		 Does not start the IWDG (independent watchdog) yet, so slow startup can't cause a reset loop.
@return  true on success, false on failure
*/
bool Watchdog_Init(void);


/*
@brief 	Register a task for monitoring. Call once per task at startup.
@param	config this Channel's setting. Cannot be NULL.
@return	A handle for all later calls, or WATCHDOG_INVALID_HANDLE on failure.
*/
WatchdogHandle_t Watchdog_Register(const Watchdog_ChannelConfig_t *config);


/*
@brief	Turns Watchdog on. Called once, after every subsystem has registered.
		Resets every channel's 'last-kick time' to "now" and starts IWDG; once started cannot be stopped on stm32. 
		Channels registered after Start causes fault action to run.
*/
bool Watchdog_Start(void);


/*
@brief	Prove the task is alive. Call from the task's own loop.
@param handle Value returned by Watchdog_Register()
Note: passing Invalid or deregistered handle is ignored --> no effect.
*/
void Watchdog_Kick(WatchdogHandle_t handle);


/*
@brief	Stop monitoring a channel (before controlled shutdown) to avoid triggering false timeout.
@param	handle Value returned by Watchdog_Register()
*/
void Watchdog_Deregister(WatchdogHandle_t handle);


/*
@brief	health of one channel. Lets DISPLAY / TELEMETRY show which subsystem is down when teh fault action is indicated rather than immediate reset.
*/
Watchdog_ChannelStatus_t Watchdog_GetChannelStatus(WatchdogHandle_t handle);


/*
@brief	true if every registered channel is currently OK; useful for startup self-check report (use case 00)
*/
bool Watchdog_IsHealthy(void);

/*
@brief	Answers why the board last reset. Valid after Watchdog_Init(). Use case 9: DISPLAY / TELEMETRY can report "recovered from watchdog reset" at startup.
*/
WatchdogResetReason_t Watchdog_GetResetReason(void);



/* SUGGESTED TIMEOUTS FOR OWNERS TO CONFIRM
   ------------------------------------------------------------------
Subsystem   Loop / period driver                 Suggested timeout_ms   Owner
---------   ----------------------------------   --------------------   -----
SENSOR x3   RT1: sample >= 4 Hz (<= 250 ms)      500					Ahmed/Brandon
FILTER      event-driven; kick on queue timeout  500					Vincent
ALERT       RT2: alert <= 500 ms; kick on queue  500					Jose
            timeout (<= 250 ms)
DISPLAY     RT6: refresh >= 2 Hz; loop wakes     500					Christian
TELEMETRY   RT7: emit >= 1 Hz; loop wakes every  500					Ed
            250 ms, sends every 4th pass         
COMMAND     event-driven (UART receive)          500					John
 
 Rule of thumb: timeout_ms >= 2x the task's kick period, and always
 <= WATCHDOG_MAX_TIMEOUT_MS.
 */

#endif /* WATCHDOG_H */