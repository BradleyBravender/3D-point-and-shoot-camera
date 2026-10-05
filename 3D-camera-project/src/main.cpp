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
// PROTOTYPES
///////////////////////////////////////////////////////////////////////////////

static void adcTask(void *);
static void stateMachineTask(void *);
static void adcTimerCallback(TimerHandle_t xTimer);
static state_t stateShootOption(button_t buttonEvt);
static state_t stateTakePhoto(button_t buttonEvt);
static state_t stateShareOption(button_t buttonEvt);
static state_t stateShare(button_t buttonEvt);
static state_t stateSettingsOption(button_t buttonEvt);
static state_t stateReviewOption(button_t buttonEvt);
static state_t stateDisplayPastPhoto(button_t buttonEvt);
static state_t stateDeletePhoto(button_t buttonEvt);
static const char* stateToString(state_t s);


///////////////////////////////////////////////////////////////////////////////
// TASKS
///////////////////////////////////////////////////////////////////////////////

static void adcTask(void *) {
    while (1) {
        // Wait for a notification from the timer (blocks indefinitely)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        button_t buttonEvt = getPressedButton();

        // Don't push no button pressed events to the queue
        if (buttonEvt == button_t::EVT_NO_BUTTON_PRESSED) {
            continue;
        }

        // ESP_LOGI(TAG, "Button pressed: %d", buttonEvt);

        if (adcDataQueue != NULL) {
            /* From https://www.freertos.org/Documentation/02-Kernel/04-API-references/06-Queues/03-xQueueSend:
            queue handle, pointer to item placed on queue, max time to wait for
            queue to become available before dropping a sample */
            if (xQueueSend(adcDataQueue, &buttonEvt, pdMS_TO_TICKS(10)) != pdPASS) {
                ESP_LOGW(TAG, "ADC queue full. Dropping sample %d", buttonEvt);
            }
        }
    }
}


static void stateMachineTask(void *) {
    button_t buttonEvt;
    state_t currentState = state_t::STATE_SHOOT_OPTION; 

    while (1) {
        ESP_LOGI(TAG, "Current state: %s", stateToString(currentState));
        
        if (xQueueReceive(adcDataQueue, &buttonEvt, pdMS_TO_TICKS(10)) == pdPASS) {
            switch (currentState) {
                case (state_t::STATE_SHOOT_OPTION):
                    currentState = stateShootOption(buttonEvt);

                    if (currentState == state_t::STATE_TAKE_PHOTO) {
                        ESP_LOGI(TAG, "Current state: %s", stateToString(currentState));
                        currentState = stateTakePhoto(buttonEvt);
                    }
                    break;
                
                case (state_t::STATE_SHARE_OPTION):
                    currentState = stateShareOption(buttonEvt);

                    if (currentState == state_t::STATE_SHARE) {
                        ESP_LOGI(TAG, "Current state: %s", stateToString(currentState));
                        currentState = stateShare(buttonEvt);
                    }
                    break;
                
                case (state_t::STATE_SETTINGS_OPTION):
                    currentState = stateSettingsOption(buttonEvt);
                    break;
                
                case (state_t::STATE_REVIEW_OPTION):
                    currentState = stateReviewOption(buttonEvt);
                    break;
                
                case (state_t::STATE_DISPLAY_PAST_PHOTO):
                    currentState = stateDisplayPastPhoto(buttonEvt);

                    if (currentState == state_t::STATE_DELETE_PHOTO) {
                        ESP_LOGI(TAG, "Current state: %s", stateToString(currentState));
                        currentState = stateDeletePhoto(buttonEvt);
                    }
                    break;

                default:
                    ESP_LOGW(TAG, "Unhandled state: %s", stateToString(currentState));
                    currentState = state_t::STATE_SHOOT_OPTION;
                    break;
            } 
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

static void adcTimerCallback(TimerHandle_t xTimer) {
    // Notify the ADC task to take a sample
    if (adcTaskHandle != NULL) {
        xTaskNotifyGive(adcTaskHandle);
    }
}


static state_t stateShootOption(button_t buttonEvt) {
    switch (buttonEvt) {
        case (button_t::EVT_BUTTON_SCROLL_D):
            return state_t::STATE_REVIEW_OPTION;
        
        case (button_t::EVT_BUTTON_SCROLL_U):
            return state_t::STATE_SHARE_OPTION;
        
        case (button_t::EVT_BUTTON_BACK):
            return state_t::STATE_SHOOT_OPTION;
        
        case (button_t::EVT_BUTTON_SELECT):
            return state_t::STATE_TAKE_PHOTO;

        default:
            return state_t::STATE_SHOOT_OPTION;
    }
}


static state_t stateTakePhoto(button_t buttonEvt) {
    // TODO: take photo
    return state_t::STATE_SHOOT_OPTION;
}


static state_t stateShareOption(button_t buttonEvt) {
    switch (buttonEvt) {
        case (button_t::EVT_BUTTON_SCROLL_D):
            return state_t::STATE_SHOOT_OPTION;
        
        case (button_t::EVT_BUTTON_SCROLL_U):
            return state_t::STATE_SETTINGS_OPTION;
        
        case (button_t::EVT_BUTTON_BACK):
            return state_t::STATE_SHARE_OPTION;
        
        case (button_t::EVT_BUTTON_SELECT):
            return state_t::STATE_SHARE;
            break;

        default:
            return state_t::STATE_SHARE_OPTION;
    }
}


static state_t stateShare(button_t buttonEvt) {
    // TODO: share photos. Add a way to cancel sharing asynchronously
    return state_t::STATE_SHARE_OPTION;
}


static state_t stateSettingsOption(button_t buttonEvt) {
    switch (buttonEvt) {
        case (button_t::EVT_BUTTON_SCROLL_D):
            return state_t::STATE_SHARE_OPTION;
        
        case (button_t::EVT_BUTTON_SCROLL_U):
            return state_t::STATE_REVIEW_OPTION;
        
        case (button_t::EVT_BUTTON_BACK):
            return state_t::STATE_SETTINGS_OPTION;
        
        case (button_t::EVT_BUTTON_SELECT):
            // TODO: add a settings option
            return state_t::STATE_SETTINGS_OPTION;

        default:
            return state_t::STATE_SETTINGS_OPTION;
    }
}


static state_t stateReviewOption(button_t buttonEvt) {
    switch (buttonEvt) {
        case (button_t::EVT_BUTTON_SCROLL_D):
            return state_t::STATE_SETTINGS_OPTION;
        
        case (button_t::EVT_BUTTON_SCROLL_U):
            return state_t::STATE_SHOOT_OPTION;
        
        case (button_t::EVT_BUTTON_BACK):
            return state_t::STATE_REVIEW_OPTION;
        
        case (button_t::EVT_BUTTON_SELECT):
            return state_t::STATE_DISPLAY_PAST_PHOTO;

        default:
            return state_t::STATE_REVIEW_OPTION;
    }
}


static state_t stateDisplayPastPhoto(button_t buttonEvt) {
    switch (buttonEvt) {
        case (button_t::EVT_BUTTON_SCROLL_D):
            // TODO: scroll down
        
        case (button_t::EVT_BUTTON_SCROLL_U):
        // TODO: scroll up
            return state_t::STATE_DISPLAY_PAST_PHOTO;
        
        case (button_t::EVT_BUTTON_BACK):
            return state_t::STATE_REVIEW_OPTION;
        
        case (button_t::EVT_BUTTON_SELECT):
            return state_t::STATE_DELETE_PHOTO;

        default:
            return state_t::STATE_DISPLAY_PAST_PHOTO;
    }
}


static state_t stateDeletePhoto(button_t buttonEvt) {
    // TODO: delete current photo
    return state_t::STATE_DISPLAY_PAST_PHOTO;
}


static const char* stateToString(state_t s) {
    switch (s) {
        case state_t::STATE_SHOOT_OPTION: return "STATE_SHOOT_OPTION";
        case state_t::STATE_TAKE_PHOTO: return "STATE_TAKE_PHOTO";
        case state_t::STATE_SHARE_OPTION: return "STATE_SHARE_OPTION";
        case state_t::STATE_SHARE: return "STATE_SHARE";
        case state_t::STATE_SETTINGS_OPTION: return "STATE_SETTINGS_OPTION";
        case state_t::STATE_REVIEW_OPTION: return "STATE_REVIEW_OPTION";
        case state_t::STATE_DISPLAY_PAST_PHOTO: return "STATE_DISPLAY_PAST_PHOTO";
        case state_t::STATE_DELETE_PHOTO: return "STATE_DELETE_PHOTO";
        default: return "UNKNOWN";
    }
}

///////////////////////////////////////////////////////////////////////////////
// ENTRY-POINT
///////////////////////////////////////////////////////////////////////////////

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