/*
readable names: turn #s like 3 and 2 into words like 'FILTER' and 'TIMED_OUT'
expected output: a different line every 250 ms, cycling through
 the whole list and then starting over, for example:
	[HH] watchdog t=1000 ch=0 (SENSOR1) status=1 since_kick=120
	[HH] watchdog t=1250 ch=255 (INVALID) status=99 (BOGUS) since_kick=80
	[HH] watchdog t=1500 ch=2 (SENSOR3) status=2 (TIMED_OUT) since_kick=640
	[HH] watchdog t=1000 ch=0 (SENSOR1)  status=1 (OK) since_kick=120     <- back to the start
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hh_platform.h"
#include "hh_watchdog.h"

//blank form
typedef struct {
    uint32_t                 timestamp_ms; //field_1
    WatchdogHandle_t         handle;	   //field_2
    Watchdog_ChannelStatus_t status;	   //field_3
    uint32_t                 since_kick_ms;//field_4
} HH_WatchdogMsg_t;

//blank form to be filled with fake data output by subsystem 'watchdog'
static const HH_WatchdogMsg_t k_script[] = {//storing as a list; compiler counts the items; list name: 'k_script'
											//'static' -> only this file can see the 'k_script' list
											//'const' -> ensures the list never changes while the firmware runs

	//each row is one filled-in HH_WatchdogMsg_t with the same order as the struct's fields: 'timestamp_ms', 'handle', 'status', 'since_kick_ms'
	//refer to: hh_watchdog.h for the possible 'status' values (health of the of a channel)
	//'handle': fake channel number (0 to 9), chosen by hand
    { 1000u, 0, WATCHDOG_CH_OK,        120u },//normal, healthy
    { 1250u, 1, WATCHDOG_CH_OK,         80u },//normal, healthy
    { 1500u, 2, WATCHDOG_CH_TIMED_OUT, 640u },//channel 2 missed its kick
    { 1750u, 3, WATCHDOG_CH_UNUSED,      0u },//channel 3: no task registered
	{ 2000u, WATCHDOG_INVALID_HANDLE, WATCHDOG_CH_OK, 100u}, //invalid handle (0xFF): not a real channel; what Watchdog_Register returns when registration fails
    { 2500u, 4, (Watchdog_ChannelStatus_t)99, 100u}, //impossible status received, only 'status'r options are: 0(UNUSED), 1(OK), 2(TIMED_OUT); '99' isn't one of them
	{ 2850u, 5, WATCHDOG_CH_OK, 4000000000u},//extremely large gap ~46 days
};

// Below answers: How many rows are in the list?  sizeof(whole list) / sizeof(one row).
//If you add or remove rows, this count updates itself.
#define SCRIPT_LEN  (sizeof(k_script) / sizeof(k_script[0]))//has to be defined after k_script exists (above)

//CHANNEL NAMES
//'const char *': "text"
static const char * const k_name[] = {//storing as a list; compiler counts the items; list name: 'k_name'
		//'static' -> only this file can see the 'k_name' list
		//'const' -> ensures the list never changes while the firmware runs
		"SENSOR1","SENSOR2","SENSOR3","FILTER","ALERT","DISPLAY","TELEMETRY","COMMAND"
};
//below answers: how many rows in 'k_name' list
#define NAME_COUNT (sizeof(k_name) / sizeof(k_name[0]))//has to be defined after k_name exists (above); used for looping back to i'th channel later

//ID's the channel the report is about
static const char *name_str(WatchdogHandle_t h){
	if (h == WATCHDOG_INVALID_HANDLE){//check_1: reserved no channel value (255)
		return "INVALID";
	}
	if (h < NAME_COUNT){//check_2: 'h' inside 'k_name' list?
		return k_name[h];			//'yes': hand back item's text
	}
	return "BAD_CH";
}

//evaluating the health of channels
static const char *status_str(Watchdog_ChannelStatus_t s){
    switch (s){//switch (s): looks at value of 's' and jumps to its matching 'case' below
    								//'return': gives answer back and leaves fcn immediately
    	//'default': catches value that matches no case
        case WATCHDOG_CH_UNUSED:    return "UNUSED";
        case WATCHDOG_CH_OK:        return "OK";
        case WATCHDOG_CH_TIMED_OUT: return "TIMED_OUT";
        default:                    return "BAD_STATUS";
    }
}


//QUEUE AND DROP COUNTER
//watchdogQ: handle that other code uses to find the queue; starts as 'NULL' -> "no queue yet"
//s_dropped: counts reports that did not fit; 'static' keep it private to this file
QueueHandle_t watchdogQ = NULL;
static uint32_t s_dropped = 0u;

void HH_Watchdog_Stub_Task(void *pv){
    (void)pv;
    hh_println("[HH] watchdog stub up");//runs once b/c outside 4loop
    watchdogQ = xQueueCreate(8,sizeof(HH_WatchdogMsg_t));//8 possible reports in HH_WatchdogMsg_t form can wait in the queue at once

    //-DEFENSIVE CHECK- that answers: "Did queue get created?"
    	//Error branch: if FreeRTOS could not allocate the queue, 'watchdogQ' is NULL: reports it and stop here (the delay keeps other tasks running)

    if (watchdogQ == NULL){
        hh_println("[HH] watchdog queue create failed");
        for (;;){
        	vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    uint32_t i = 0u; // i = which row we are on. Rows count initialized at 0 (0 = first row)
    HH_WatchdogMsg_t msg;//variable defined of the form: datatype strub w/4 fields to be filled

    for (;;){
    	msg = k_script[i];//take one row i from k_script list and put a copy into msg

    	//send a copy of msg into the queue; 0 = don't wait if the queue is full
    	//'pdPASS' = it went in; anything else = queue was full, so count it as dropped
    	if (xQueueSend(watchdogQ, &msg, 0) != pdPASS){
    		s_dropped++;
    	}

    	//print the 4 fields of data struct 'HH_WatchdogMsg_t' in one line: #, text, text, #
    	hh_print("[HH] watchdog t=");      	    //<- fixed text
    	hh_print_u32(msg.timestamp_ms);    // <- the number stored in field 1
    	hh_print(" ch=");						//<- fixed text
//hh_print_u32 only accepts the type uint32_t, but 'handle' and 'status' are different types. Put (uint32_t) in front to convert them; this conversion step is called 'type casting'
    	hh_print_u32((uint32_t)msg.handle);// <- the value stored in field 2
    	hh_print(" (");							//<- fixed text: opening parenthesis
    	hh_print(name_str(msg.handle));			//<- the channel's name, as text
    	hh_print(")");							//<- fixed text: closing parenthesis
    	hh_print(" status=");					//<- fixed text
//hh_print_u32 only accepts the type uint32_t, but 'handle' and 'status' are different types. Put (uint32_t) in front to convert them; this conversion step is called 'type casting'
    	hh_print_u32((uint32_t)msg.status);// <- the value stored in field 3
    	hh_print(" (");							//<- fixed text: opening parenthesis
    	hh_print(status_str(msg.status));		//<- the status, as text
    	hh_print(")");							//<- fixed text: closing parenthesis
    	hh_print(" since_kick=");				//<- fixed text
    	hh_print_u32(msg.since_kick_ms);		// <- the number stored in field 4
    	hh_print(" dropped=");					//<- fixed text
    	hh_print_u32(s_dropped);				// <- how many reports did not fit in the queue so far
    	hh_println(""); //line break w/no print output -> new line

    	//move to the next row, wrapping to 0 after last one
    	i = (i + 1u) % SCRIPT_LEN;//i increments by 1, and is divided by script length. when remainder =0, aka numerator and denominator are same, 'i' loops back to 0
    	vTaskDelay(pdMS_TO_TICKS(250));//sleep 250 ms,since the loop runs w/out pause, the 'vTaskDelay' serves as that blocked state (sleep) to allow other tasks to use CPU and execute
    }
}
