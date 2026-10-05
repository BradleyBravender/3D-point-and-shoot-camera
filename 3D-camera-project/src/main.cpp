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

#include "system_config.hpp"
#include "buttons.hpp"
#include "display.hpp"
#include "image_sensor.hpp"
#include "sd_card.hpp"

///////////////////////////////////////////////////////////////////////////////
// GLOBALS
///////////////////////////////////////////////////////////////////////////////

#define ADC_TIMER_PERIOD_MS 100
#define QUEUE_LENGTH        10

// Priorities
#define PRIO_ADC_TASK       5
#define PRIO_SM_TASK        5

// Stack (in words)
#define STACK_ADC_TASK      2048
#define STACK_SM_TASK       2048

// Handles
static TaskHandle_t adcTaskHandle   = NULL;
static TaskHandle_t stateMachineTaskHandle = NULL;
static TimerHandle_t adcTimerHandle = NULL;
static QueueHandle_t adcDataQueue   = NULL;

static const char* TAG = "MainModule";

///////////////////////////////////////////////////////////////////////////////
// TASKS
///////////////////////////////////////////////////////////////////////////////

static void adcTask(void *) {
    while (1) {
        // Wait for a notification from the timer (blocks indefinitely)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        button_t buttonEvt = getPressedButton();

        ESP_LOGI(TAG, "Button pressed: %d", buttonEvt);

        if (adcDataQueue != NULL) {
            /* From https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/03-xQueueSend:
            queue handle, pointer to item placed on queue, max time to wait for
            queue to become available before dropping a sample */
            if (xQueueSend(adcDataQueue, &buttonEvt, pdMS_TO_TICKS(10)) != pdPASS) {
                // ESP_LOGW(TAG, "ADC queue full. Dropping sample %d", buttonEvt);
            }
        }
    }
}


static void stateMachineTask(void *) {
    button_t buttonEvt;
    
    while (1) {
        if (xQueueReceive(adcDataQueue, &buttonEvt, pdMS_TO_TICKS(10)) != pdPASS) {
                // ESP_LOGW(TAG, "ADC queue empty.");
            }

        const char *buttonType = "123456789";
        
        switch (buttonEvt) {
            case button_t::EVT_BUTTON_SCROLL_D:
                buttonType = "Scroll D";
                break;
            case button_t::EVT_BUTTON_SCROLL_U:
                buttonType = "Scroll U";
                break;
            case button_t::EVT_BUTTON_BACK:
                buttonType = "Back";
                break;
            case button_t::EVT_BUTTON_SELECT:
                buttonType = "Select";
                break;
            case button_t::EVT_NO_BUTTON_PRESSED:
                buttonType = "No Press";
                break;
        }
        
        ESP_LOGI(TAG, "Received: %s", buttonType);
    }
}


///////////////////////////////////////////////////////////////////////////////
// NON-TASK FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

static void adcTimerCallback(TimerHandle_t xTimer) {
    // Notify the ADC task to take a sample
    if (adcTaskHandle != NULL) {
        xTaskNotifyGive(adcTaskHandle);
    }
}


extern "C" void app_main(void) {
    buttonsInit();

    // Create queue for ADC button events
    adcDataQueue = xQueueCreate(QUEUE_LENGTH, sizeof(button_t));
    if (adcDataQueue == NULL) {
        ESP_LOGE(TAG, "Failed to create ADC queue");
        return;
    }

    // Create tasks
    BaseType_t ret;

    /* From https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/01-xTaskCreate:
    task function, descriptive name, # of words to allocate for the tasks' stack,
    parameters to pass to the task, priority, task handle (helpful for passing notifications) */
    ret = xTaskCreate(adcTask, "ADC_Task", STACK_ADC_TASK, NULL, PRIO_ADC_TASK, &adcTaskHandle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create ADC task");
        return;
    }

    ret = xTaskCreate(stateMachineTask, "State_Machine_Task", STACK_SM_TASK, NULL, PRIO_SM_TASK, &stateMachineTaskHandle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create State Machine task");
        return;
    }

    /* From https://www.freertos.org/Documentation/02-Kernel/04-API-references/11-Software-timers/01-xTimerCreate:
    human-readable name, timer period, one-shot (false) or repeating (true),
    an ID for the timer, the function to call when the timer expires */
    adcTimerHandle = xTimerCreate(
        "ADC_Timer",
        pdMS_TO_TICKS(ADC_TIMER_PERIOD_MS),
        pdTRUE,
        NULL,
        adcTimerCallback
    );
    if (adcTimerHandle == NULL) {
        ESP_LOGE(TAG, "Failed to create ADC timer");
        return;
    }

    if (xTimerStart(adcTimerHandle, pdMS_TO_TICKS(100)) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start ADC timer");
        return;
    }

    ESP_LOGI(TAG, "Setup complete. Timer started with %d ms period", ADC_TIMER_PERIOD_MS);  
}