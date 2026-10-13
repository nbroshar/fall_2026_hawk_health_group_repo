/*	CENG4200 - Embedded Systems
	Command Module - Header File Final Varient
	last Update Date - 9/30
	Author:  John Plath



	[Requirments]
	> Has to be able to accept requests from the display
	> Has to be able to accept 'hot swap' style runtime change requests [from the following modules]
		- Sensors
		- Alert
			> The user has to be able to change the threshold of these two | dynamic
	> Has to be able to identify origin of the runtime change request
	> Has to be able to identify to identify input or change request that is out of bounds

	[Documentation Reference]
	> Please review 'command-header-and-main.doc' for notes on the code | This will be sent on teams for the group to read
		- it takes up waaaaaaay to much room on this file
*/

// (if applicable) - Create a queue for incoming
// Create the main function call

typedef struct {
	// pyt required types here
} Command_Handler_t; // This 'should' act as the types declaration

// [Experimental] - 'Command_Initializer' -- checks if config is active
void Command_Initializer(void);

// [Also Experimental] - 'Command_DataLine' -- Feed from the module requesting the change | Accept character | accept symbols | accept numbers
void Command_DataLine(char c);

// [Ready] - 'Command_TaskHandler' -- handles the change request | Ability to read the input, parse data types, create buffer | Side Note - NOT STATIC, its DYNAMIC
void Command_TaskHandler(void *pv);

// [Ready, planned] - 'Command_TaskHandler_Error' handles errors that stem from transfering input data, communication with hung modules
void Command_TaskHandler_Error(void *pv);


// [Breif Descriptions]
// Command_Initializer - Should be called by config alone

// Command_DataLine - Should be called by modules trying to send an input request, Threshold change, runtime change request

// Command_TaskHandler - Internal Function | Should be used to read the input type or request and the current runtime environment | Results sent through data line

// Command_TaskHandler_Error - this is an internal function only, should be used to detect errors and respond accordingly
