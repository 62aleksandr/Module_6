#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

// Define the GPIO pin for the LED
constexpr gpio_num_t BLINK_15 = GPIO_NUM_15;
constexpr gpio_num_t BLINK_16 = GPIO_NUM_16;
constexpr gpio_num_t BLINK_17 = GPIO_NUM_17;

constexpr BaseType_t xCoreID = 1;

TaskHandle_t led15_task_handle = NULL;
TaskHandle_t led16_task_handle = NULL;
TaskHandle_t led17_task_handle = NULL;

// Task for LED 15
void led15_task(void *pvParameters)
{
	for (;;)
	{
		gpio_set_level(BLINK_15, 1);
		vTaskDelay(pdMS_TO_TICKS(500));

		gpio_set_level(BLINK_15, 0);
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

// Task for LED 16
void led16_task(void *pvParameters)
{
	for (;;)
	{
		gpio_set_level(BLINK_16, 1);
		vTaskDelay(pdMS_TO_TICKS(300));

		gpio_set_level(BLINK_16, 0);
		vTaskDelay(pdMS_TO_TICKS(300));
	}
}

// Task for LED 17
void led17_task(void *pvParameters)
{
	for (;;)
	{
		gpio_set_level(BLINK_17, 1);
		vTaskDelay(pdMS_TO_TICKS(200));

		gpio_set_level(BLINK_17, 0);
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

extern "C" void app_main(void)
{
	// Configure the GPIO pin
	// gpio_reset_pin(BLINK_GPIO);
	// gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << BLINK_15) | (1ULL << BLINK_16) | (1ULL << BLINK_17),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE};
	gpio_config(&io_conf);

	// Create tasks
	xTaskCreatePinnedToCore(
		led15_task,			// функція задачі
		"LED15_Task",		// назва задачі
		1024,				// розмір стека, байт
		NULL,				// параметри, що передаються задачі
		5,					// пріоритет задачі
		&led15_task_handle, // дескриптор задачі
		xCoreID);			// ядро процесора
	xTaskCreatePinnedToCore(
		led16_task,
		"LED16_Task",
		1024,
		NULL,
		5,
		&led16_task_handle,
		xCoreID);
	xTaskCreatePinnedToCore(
		led17_task,
		"LED17_Task",
		1024,
		NULL,
		5,
		&led17_task_handle,
		xCoreID);
}
