#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "temp_task.h"
#include "oled_task.h"
#include "rtc_task.h"
#include "i2c_bus.h"
#include "app_data.h"

// Queue parameters
#define QUEUE_LENGTH 5
#define QUEUE_ITEM_SIZE sizeof(app_data_t)

task_context_t task_context;

void sensors_init(void)
{
	i2c_init();
	ds18b20_init();
	oled_dev_init();
	rtc_dev_init();
}

void tasks_init(void)
{
	// Create Queue
	task_context.data_queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);

	if (task_context.data_queue == NULL)
	{
		return;
	}

	// Create Mutex
	task_context.i2c_mutex = xSemaphoreCreateMutex();

	if (task_context.i2c_mutex == NULL)
	{
		return;
	}

	// Create Tasks
	xTaskCreate(temp_task, "temp_task", 4096, &task_context, 5, NULL);
	xTaskCreate(rtc_task, "rtc_task", 4096, &task_context, 5, NULL);
	xTaskCreate(oled_task, "oled_task", 4096, &task_context, 4, NULL);
}