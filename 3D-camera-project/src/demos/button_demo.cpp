/*  
  Rui Santos & Sara Santos - Random Nerd Tutorials
  https://RandomNerdTutorials.com/esp-idf-esp32-gpio-analog-adc/
*/

#include <freertos/FreeRTOS.h>      // Provides the core FreeRTOS types and functions;
#include <freertos/task.h>          // Allows to use of the non-blocking delay function vTaskDelay;
#include <esp_adc/adc_oneshot.h>    // Includes the functions required to configure the ESP32’s ADC in One-Shot mode to take analog readings from specific channels;
#include <esp_log.h>                // Offers a framework to format log messages in the serial monitor for debugging;
#include "sdkconfig.h"              // Includes the project’s configuration file.

#define ADC_PIN       ADC_CHANNEL_0     // Channel 7 - Check ESP32 Pinout for the GPIO Number
#define ADC_UNIT      ADC_UNIT_2        // ADC1
#define ADC_BITWIDTH  ADC_BITWIDTH_12   // 12-bit resolution (0-4095)
#define ADC_ATTEN     ADC_ATTEN_DB_12   // ~3.3V full-scale voltage

extern "C" void app_main(void)
{
    int adc_value;
    adc_oneshot_unit_handle_t adc_handle;

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

    // ADC Oneshot Analog Read loop
    while (1) {
        // Read ADC value with Oneshot
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_PIN, &adc_value));
        // Print ADC value
        ESP_LOGI("ADC Value", "%d", adc_value);
        // Delay 1 second
        vTaskDelay(1000 / portTICK_PERIOD_MS); 
    }
}
