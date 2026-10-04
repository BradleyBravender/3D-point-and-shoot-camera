#include "buttons.hpp"

/* Create functions to read from the ADC and return the pressed button */

void buttons_init() {

}


button_t getPressedButton() {
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_PIN, &adc_value));
    ESP_LOGI("ADC Value", "%d", adc_value);
    
    if (isInRange(adc_value, static_cast<int>(button_t::EVT_BUTTON_SCROLL_D))) {
        return button_t::EVT_BUTTON_SCROLL_D;
    } else if (isInRange(adc_value, static_cast<int>(button_t::EVT_BUTTON_SCROLL_U))) {
        return button_t::EVT_BUTTON_SCROLL_U;
    } else if (isInRange(adc_value, static_cast<int>(button_t::EVT_BUTTON_BACK))) {
        return button_t::EVT_BUTTON_BACK;
    } else if (isInRange(adc_value, static_cast<int>(button_t::EVT_BUTTON_SELECT))) {
        return button_t::EVT_BUTTON_SELECT;
    } else {
        return button_t::EVT_NO_BUTTON_PRESSED;
    }
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