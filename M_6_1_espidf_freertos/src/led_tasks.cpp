#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "led_tasks.h"

static const char *TAG = "MAIN";

constexpr BaseType_t xCoreID = 1;

TaskHandle_t led15_task_handle = NULL;
TaskHandle_t led16_task_handle = NULL;
TaskHandle_t led17_task_handle = NULL;

// Task 1
void task_1(void *pvParameters)
{
	TickType_t last_tick = xTaskGetTickCount();
	uint32_t loop_counter = 0;

	for (;;)
	{
		loop_counter++;
		TickType_t current_tick = xTaskGetTickCount();

		// Как только тик системного таймера изменился, выводим информацию
		if (current_tick != last_tick)
		{
			ESP_LOGI(TAG, "Task_1. Loops done: %lu at Tick: %lu",
					 (unsigned long)loop_counter, (unsigned long)current_tick);

			loop_counter = 0; // Сбрасываем счетчик для следующего кванта времени
			last_tick = current_tick;
		}
	}
}

// Task 2
void task_2(void *pvParameters)
{
	TickType_t last_tick = xTaskGetTickCount();
	uint32_t loop_counter = 0;

	for (;;)
	{
		loop_counter++;
		TickType_t current_tick = xTaskGetTickCount();

		// Как только тик системного таймера изменился, выводим информацию
		if (current_tick != last_tick)
		{
			ESP_LOGI(TAG, "Task_2. Loops done: %lu at Tick: %lu",
					 (unsigned long)loop_counter, (unsigned long)current_tick);

			loop_counter = 0; // Сбрасываем счетчик для следующего кванта времени
			last_tick = current_tick;
		}
	}
}

// Task 3
void task_3(void *pvParameters)
{
	TickType_t last_tick = xTaskGetTickCount();
	uint32_t loop_counter = 0;

	for (;;)
	{
		loop_counter++;
		TickType_t current_tick = xTaskGetTickCount();

		// Как только тик системного таймера изменился, выводим информацию
		if (current_tick != last_tick)
		{
			ESP_LOGI(TAG, "Task_3. Loops done: %lu at Tick: %lu",
					 (unsigned long)loop_counter, (unsigned long)current_tick);

			loop_counter = 0; // Сбрасываем счетчик для следующего кванта времени
			last_tick = current_tick;
		}
	}
}

void create_led_tasks()
{
	// Create tasks
	xTaskCreatePinnedToCore(
		task_1,				// функція задачі
		"task_1",			// назва задачі
		4096,				// розмір стека, байт
		NULL,				// параметри, що передаються задачі
		5,					// пріоритет задачі
		&led15_task_handle, // дескриптор задачі
		xCoreID);			// ядро процесора

	xTaskCreatePinnedToCore(
		task_2,
		"task_2",
		4096,
		NULL,
		5,
		&led16_task_handle,
		xCoreID);

	xTaskCreatePinnedToCore(
		task_3,
		"task_3",
		4096,
		NULL,
		5,
		&led17_task_handle,
		xCoreID);
}

void led_tasks_init()
{

	create_led_tasks();
}
