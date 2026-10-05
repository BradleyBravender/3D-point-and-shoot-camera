#pragma once

///////////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////////

#include <freertos/FreeRTOS.h>      
#include <freertos/task.h>           
#include "freertos/queue.h"
#include "freertos/timers.h"  

#include <esp_adc/adc_oneshot.h> 
#include <esp_log.h>
#include <esp_err.h>

#include "sdkconfig.h"  

///////////////////////////////////////////////////////////////////////////////
// PIN CONFIGS
///////////////////////////////////////////////////////////////////////////////

#define ADC_PIN       ADC_CHANNEL_0     // See ESP32 Pinout for GPIO Number
#define ADC_UNIT      ADC_UNIT_2        // ADC2
#define ADC_BITWIDTH  ADC_BITWIDTH_12   // 12-bit resolution (0-4095)
#define ADC_ATTEN     ADC_ATTEN_DB_12   // ~3.3V full-scale voltage

///////////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////////

enum class state_t {
    STATE_SHOOT_OPTION,
    STATE_TAKE_PHOTO,
    STATE_SHARE_OPTION,
    STATE_SHARE,
    STATE_SETTINGS_OPTION, // TODO: will need to add a settings state machine
    STATE_REVIEW_OPTION,
    STATE_DISPLAY_PAST_PHOTO,
    STATE_DELETE_PHOTO
};
