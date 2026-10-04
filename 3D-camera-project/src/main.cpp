/*
This file will execute the overarching state machine. It should include files 
of helper functions/classes for the image sensor, sd card management, etc.

The tasks are as follows:
* Reading image sensor data and writing it to the display
    * Should this be split into 2 tasks?
* Listening for, and adding button events to a queue
    * Because the input is an ADC, implement polling
* Managing state transitions and dispatching accordingly
    * Based on the most recent button press, 

Introduction example:
* If button 1 is pressed), go from menu to share
* If if button 2 is pressed, go from share to menu
 ________       ________
|        |____>|        |
| menu   |<____| share  |
|________|     |________|

*/

///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "buttons.hpp"

///////////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////////

enum class button_t {
    EVT_BUTTON_1,
    EVT_BUTTON_2
};

enum class state_t {
    STATE_MENU,
    STATE_SHARE,
};

///////////////////////////////////////////////////////////////////////////////
// TASKS
///////////////////////////////////////////////////////////////////////////////

/*
TODO: create a button task
- poll the ADC
*/

/* 
TODO: create a state machine task
*/


