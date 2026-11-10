/* building one "report" by hand and printing it
 *
 /*expected output: :
 [HH] watchdog t=1000 ch=3 status=1 since_kick=120
 * It repeats every 250 ms.
 */
//Report says: "At time 1000 ms, channel 3 had status 1, and its last kick was 120 ms ago."



 /* Below: provides two types the struct needs:
	WatchdogHandle_t          = the type for "which channel"
	Watchdog_ChannelStatus_t  = the type for "OK / TIMED_OUT / UNUSED"
*/
#include "FreeRTOS.h"
#include "task.h"
#include "hh_platform.h"
#include "hh_watchdog.h"

//define the struct
typedef struct {
    uint32_t                 timestamp_ms;   // field_1: in ms, clock reading of when subsystem Watchdog made the report; time of day-> always growing
    WatchdogHandle_t         handle;         //field_2: channel number (0 = first one)
    Watchdog_ChannelStatus_t status;         //field_3: 'OK', 'TIMED_OUT', or 'UNUSED' <- defined in watchdog.h
    uint32_t                 since_kick_ms;  //field_4: ms since the last kick; length of time -> grows until a kick, then drops back to 0
} HH_WatchdogMsg_t;							//struct definition

void HH_Watchdog_Stub_Task(void *pv){
    (void)pv;//fcn's input
    HH_WatchdogMsg_t msg;//declare variable of the data type 'HH_WatchdogMsg_t', creating blank form to be filled
    hh_println("[HH] watchdog stub up");//announcing that setup is done
    //below: giving each field of the data type 'HH_WatchdogMsg_t' a value:
     msg.timestamp_ms = 1000u;//1000ms
     msg.handle = 3;		  //channel 3 selected
     msg.since_kick_ms = 120;  //120ms
     msg.status = WATCHDOG_CH_OK;

    for (;;){
        //print the 4 fields of data struct 'HH_WatchdogMsg_t' in one line: #, text, text, #
         hh_print("[HH] watchdog t=");      	 //<- fixed text
         hh_print_u32(msg.timestamp_ms);    // <- the number stored in field 1
         hh_print(" ch=");						//<- fixed text
         hh_print_u32((uint32_t)msg.handle);// <- the value stored in field 2
         hh_print(" status=");					//<- fixed text
         hh_print_u32((uint32_t)msg.status);// <- the value stored in field 3
         hh_print(" since_kick=");				//<- fixed text
         hh_print_u32(msg.since_kick_ms);		// <- the number stored in field 4
         hh_println(""); //line break w/no print output -> new line
         //hh_print_u32 only accepts the type uint32_t, but 'handle' and 'status' are different types. Put (uint32_t) in front to convert them; this conversion step is called 'type casting'
        vTaskDelay(pdMS_TO_TICKS(250));   // sleep 250 ms,since the loop runs w/out pause, the 'vTaskDelay' serves as that blocked state (sleep) to allow other tasks to use CPU and execute
    }
}
