#include "buttons.hpp"

///////////////////////////////////////////////////////////////////////////////
// PRIVATE GLOBALS
///////////////////////////////////////////////////////////////////////////////

adc_oneshot_unit_handle_t adc_handle;

///////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

void buttonsInit() {
    static const char* TAG = "Buttons";

    // Initialize ADC Oneshot Mode Driver on the ADC Unit
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    // Configure ADC channel
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_PIN, &config));

    ESP_LOGI(TAG, "ADC set up successfully");
}


button_t getPressedButton() {
    int adc_value;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_PIN, &adc_value));
    
    if (isInRange(adc_value, static_cast<int>(button_t::EVT_BUTTON_SCROLL_D))) {
        return button_t::EVT_BUTTON_SCROLL_D;
    } else if (isInRange(adc_value, static_cast<int>(button_t::EVT_BUTTON_SCROLL_U))) {
        return button_t::EVT_BUTTON_SCROLL_U;
    } else if (isInRange(adc_value, static_cast<int>(button_t::EVT_BUTTON_BACK))) {
        return button_t::EVT_BUTTON_BACK;
    } else if (isInRange(adc_value, static_cast<int>(button_t::EVT_BUTTON_SELECT))) {
        return button_t::EVT_BUTTON_SELECT;
    } 
    
    return button_t::EVT_NO_BUTTON_PRESSED;
}


bool isInRange(int measured_value, int expected_value) {
    // ! If the tolerance is increased, buttons may overlap
    int tolerance = 100;
    
    if ( ((expected_value - tolerance) <= measured_value) && 
        (measured_value <= (expected_value + tolerance)) ) {
        return true;
    }

    return false;
}