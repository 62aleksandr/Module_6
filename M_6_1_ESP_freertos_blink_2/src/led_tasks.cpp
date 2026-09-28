#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "led_tasks.h"

struct LedTaskConfig
{
	const char *name;
	gpio_num_t gpio_num;
	uint32_t on_delay_ms;
	uint32_t off_delay_ms;
};

LedTaskConfig led15_params = {"LED15_Task", GPIO_NUM_15, 500, 500};
LedTaskConfig led16_params = {"LED16_Task", GPIO_NUM_16, 300, 300};
LedTaskConfig led17_params = {"LED17_Task", GPIO_NUM_17, 200, 200};

constexpr BaseType_t xCoreID = 1;

TaskHandle_t led15_task_handle = NULL;
TaskHandle_t led16_task_handle = NULL;
TaskHandle_t led17_task_handle = NULL;

// Task for LED 15
void led15_task(void *pvParameters)
{
	LedTaskConfig *config = (LedTaskConfig *)pvParameters;

	for (;;)
	{

		gpio_set_level(config->gpio_num, 1);
		vTaskDelay(pdMS_TO_TICKS(config->on_delay_ms));

		gpio_set_level(config->gpio_num, 0);
		vTaskDelay(pdMS_TO_TICKS(config->off_delay_ms));
	}
}

// Task for LED 16
void led16_task(void *pvParameters)
{
	LedTaskConfig *config = (LedTaskConfig *)pvParameters;

	for (;;)
	{

		gpio_set_level(config->gpio_num, 1);
		vTaskDelay(pdMS_TO_TICKS(config->on_delay_ms));

		gpio_set_level(config->gpio_num, 0);
		vTaskDelay(pdMS_TO_TICKS(config->off_delay_ms));
	}
}

// Task for LED 17
void led17_task(void *pvParameters)
{
	LedTaskConfig *config = (LedTaskConfig *)pvParameters;

	for (;;)
	{

		gpio_set_level(config->gpio_num, 1);
		vTaskDelay(pdMS_TO_TICKS(config->on_delay_ms));

		gpio_set_level(config->gpio_num, 0);
		vTaskDelay(pdMS_TO_TICKS(config->off_delay_ms));
	}
}

void led_gpio_init()
{
	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << led15_params.gpio_num) | (1ULL << led16_params.gpio_num) | (1ULL << led17_params.gpio_num),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE};
	gpio_config(&io_conf);
}

void create_led_tasks()
{
	// Create tasks
	xTaskCreatePinnedToCore(
		led15_task,			// функція задачі
		led15_params.name,	// назва задачі
		2048,				// розмір стека, байт
		&led15_params,		// параметри, що передаються задачі
		5,					// пріоритет задачі
		&led15_task_handle, // дескриптор задачі
		xCoreID);			// ядро процесора

	xTaskCreatePinnedToCore(
		led16_task,
		led16_params.name,
		2048,
		&led16_params,
		5,
		&led16_task_handle,
		xCoreID);

	xTaskCreatePinnedToCore(
		led17_task,
		led17_params.name,
		2048,
		&led17_params,
		5,
		&led17_task_handle,
		xCoreID);
}

void led_tasks_init()
{
	led_gpio_init();
	create_led_tasks();
}
