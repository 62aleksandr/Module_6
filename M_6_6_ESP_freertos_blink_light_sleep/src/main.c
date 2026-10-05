// Automatic Light Sleep
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_pm.h"

// GPIO LED
#define BLINK_10 GPIO_NUM_10
#define BLINK_11 GPIO_NUM_11
#define BLINK_12 GPIO_NUM_12

// Ядро для виконання задач
#define xCoreID 1

// Дескриптори задач
TaskHandle_t led10_task_handle = NULL;
TaskHandle_t led11_task_handle = NULL;
TaskHandle_t led12_task_handle = NULL;

// Час ON/OFF — 40 секунд
#define SLEEP_TIME_MS (20 * 1000)

static const char *TAG = "MAIN";

// Задача LED10
void led10_task(void *pvParameters)
{
	for (;;)
	{
		gpio_set_level(BLINK_10, 1);
		ESP_LOGI(TAG, "LED 10 ON");
		vTaskDelay(pdMS_TO_TICKS(SLEEP_TIME_MS));

		gpio_set_level(BLINK_10, 0);
		ESP_LOGI(TAG, "LED 10 OFF");
		vTaskDelay(pdMS_TO_TICKS(SLEEP_TIME_MS));
	}
}

// Задача LED11
void led11_task(void *pvParameters)
{
	for (;;)
	{
		gpio_set_level(BLINK_11, 1);
		ESP_LOGI(TAG, "LED 11 ON");
		vTaskDelay(pdMS_TO_TICKS(SLEEP_TIME_MS));

		gpio_set_level(BLINK_11, 0);
		ESP_LOGI(TAG, "LED 11 OFF");
		vTaskDelay(pdMS_TO_TICKS(SLEEP_TIME_MS));
	}
}

// Задача LED12
void led12_task(void *pvParameters)
{
	for (;;)
	{
		gpio_set_level(BLINK_12, 1);
		ESP_LOGI(TAG, "LED 12 ON");
		vTaskDelay(pdMS_TO_TICKS(SLEEP_TIME_MS));

		gpio_set_level(BLINK_12, 0);
		ESP_LOGI(TAG, "LED 12 OFF");
		vTaskDelay(pdMS_TO_TICKS(SLEEP_TIME_MS));
	}
}

void app_main(void)
{
	// Configure the GPIO pin
	// gpio_reset_pin(BLINK_GPIO);
	// gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

	// Налаштовуємо GPIO LED
	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << BLINK_10) | (1ULL << BLINK_11) | (1ULL << BLINK_12),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE};
	gpio_config(&io_conf);

	// Налаштовуємо автоматичне керування живленням
	esp_pm_config_t pm_config = {
		.max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
		.min_freq_mhz = 40,
		.light_sleep_enable = true // ON/OFF true/false
	};
	// Налаштування Power Management
	esp_err_t err = esp_pm_configure(&pm_config);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to configure power management");
	}

	// Створюємо задачі LED
	xTaskCreatePinnedToCore(
		led10_task,			// функція задачі
		"LED10_Task",		// назва задачі
		2048,				// розмір стека, байт
		NULL,				// параметри, що передаються задачі
		5,					// пріоритет задачі
		&led10_task_handle, // дескриптор задачі
		xCoreID);			// ядро процесора

	xTaskCreatePinnedToCore(led11_task, "LED11_Task", 2048, NULL, 5, &led11_task_handle, xCoreID);

	xTaskCreatePinnedToCore(led12_task, "LED12_Task", 2048, NULL, 5, &led12_task_handle, xCoreID);
}
