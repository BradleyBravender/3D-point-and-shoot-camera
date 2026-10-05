// #include <stdio.h>
// #include <stdlib.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "freertos/timers.h"
// #include "driver/adc.h"
// #include "driver/gpio.h"
// #include "esp_log.h"
// #include "esp_err.h"

// ----- Config -----
// #define ADC_GPIO            GPIO_NUM_34      // ADC1_CH6
// #define ADC_CHANNEL         ADC1_CHANNEL_6
// #define ADC_ATTEN           ADC_ATTEN_DB_12  // For full-scale ~3.3V
// #define ADC_WIDTH           ADC_WIDTH_BIT_12

// #define LED_GPIO            GPIO_NUM_2

// #define ADC_TIMER_PERIOD_MS 1000             // ADC sample every 1000 ms
#define QUEUE_LENGTH        10

// Task stack sizes and priorities
#define STACK_ADC_TASK      2048
#define STACK_COMM_TASK     4096
#define STACK_LED_TASK      2048

#define PRIO_ADC_TASK       5
#define PRIO_COMM_TASK      6   // higher so comm can process quickly
#define PRIO_LED_TASK       3

// ----- Globals -----
static QueueHandle_t adcDataQueue = NULL;
static TaskHandle_t adcTaskHandle  = NULL;
static TaskHandle_t commTaskHandle = NULL;
static TaskHandle_t ledTaskHandle  = NULL;
static TimerHandle_t adcTimer      = NULL;

static const char *TAG = "multitask_demo";

// ----- Prototypes -----
// static void adc_init(void);
// static void adcTask(void *pvParameters);
// static void commTask(void *pvParameters);
// static void ledTask(void *pvParameters);
// static void adcTimerCallback(TimerHandle_t xTimer);

// ----- Implementation -----

static void adc_init(void)
{
    // Configure ADC1 width and channel attenuation
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN));
    ESP_LOGI(TAG, "ADC initialized on GPIO %d (ADC1_CH6)", ADC_GPIO);
}

static void adcTimerCallback(TimerHandle_t xTimer)
{
    // Timer service task context - notify ADC task to sample
    if (adcTaskHandle != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        // Use xTaskNotifyGiveFromISR if this were a real ISR; timer callback runs in timer service task so normal notify is fine:
        xTaskNotifyGive(adcTaskHandle);
    }
}

static void adcTask(void *pvParameters)
{
    (void) pvParameters;
    int raw = 0;

    while (1) {
        // Wait for notification from timer (blocks indefinitely)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Read ADC raw value
        raw = adc1_get_raw(ADC_CHANNEL);

        // Optionally convert raw to voltage using calibration (not included here).
        // Send raw value to queue (non-blocking; if full, drop oldest or handle as needed)
        if (adcDataQueue != NULL) {
            if (xQueueSend(adcDataQueue, &raw, pdMS_TO_TICKS(10)) != pdPASS) {
                ESP_LOGW(TAG, "ADC queue full. Dropping sample %d", raw);
            }
        }
        // Note: we do not notify LED here. Communication task notifies LED after successful send.
    }
}

static void commTask(void *pvParameters)
{
    (void) pvParameters;
    int recvValue = 0;

    // Example: if using Wi-Fi, initialize it here (omitted). This demo uses UART (printf).
    while (1) {
        // Block until an ADC value is available
        if (xQueueReceive(adcDataQueue, &recvValue, portMAX_DELAY) == pdPASS) {
            // Send over UART (console)
            printf("ADC Value: %d\n", recvValue);
            fflush(stdout);

            // If you want to use Wi-Fi/MQTT, call the send function here instead of printf.
            // Example:
            // if (wifi_send_value(recvValue) == ESP_OK) { ... }

            // Notify LED task about successful send
            if (ledTaskHandle != NULL) {
                xTaskNotifyGive(ledTaskHandle);
            }
        }
    }
}

static void ledTask(void *pvParameters)
{
    (void) pvParameters;

    // Configure LED pin
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0);

    while (1) {
        // Wait indefinitely for a notification (from commTask)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Blink once to indicate data sent
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(100));   // LED on for 100 ms
        gpio_set_level(LED_GPIO, 0);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 FreeRTOS Multitasking Example");

    // Initialize ADC
    adc_init();

    // Create queue for ADC data
    adcDataQueue = xQueueCreate(QUEUE_LENGTH, sizeof(int));
    if (adcDataQueue == NULL) {
        ESP_LOGE(TAG, "Failed to create ADC queue");
        return;
    }

    // Create tasks
    BaseType_t ret;

    ret = xTaskCreate(adcTask, "ADC_Task", STACK_ADC_TASK, NULL, PRIO_ADC_TASK, &adcTaskHandle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create ADC task");
        return;
    }

    ret = xTaskCreate(commTask, "Comm_Task", STACK_COMM_TASK, NULL, PRIO_COMM_TASK, &commTaskHandle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create Comm task");
        return;
    }

    ret = xTaskCreate(ledTask, "LED_Task", STACK_LED_TASK, NULL, PRIO_LED_TASK, &ledTaskHandle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create LED task");
        return;
    }

    // Create software timer to request ADC sampling periodically
    adcTimer = xTimerCreate("ADC_Timer",
                             pdMS_TO_TICKS(ADC_TIMER_PERIOD_MS),
                             pdTRUE,
                             NULL,
                             adcTimerCallback);
    if (adcTimer == NULL) {
        ESP_LOGE(TAG, "Failed to create ADC timer");
        return;
    }

    if (xTimerStart(adcTimer, pdMS_TO_TICKS(100)) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start ADC timer");
        return;
    }

    ESP_LOGI(TAG, "Setup complete. Timer started with %d ms period", ADC_TIMER_PERIOD_MS);

}