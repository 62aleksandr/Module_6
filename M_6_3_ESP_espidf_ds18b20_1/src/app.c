#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "temp_task.h"
#include "led_task.h"

// Queue parameters
#define QUEUE_LENGTH 5
#define QUEUE_ITEM_SIZE sizeof(float)

// Queue handle
QueueHandle_t temp_queue;

void sensors_init(void)
{
	ds18b20_init();
	led_init();
}

void tasks_init(void)
{
	temp_queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);

	if (temp_queue == NULL)
	{
		return;
	}

	xTaskCreate(temp_task, "temp_task", 4096, temp_queue, 5, NULL);
	xTaskCreate(led_task, "led_task", 4096, temp_queue, 4, NULL);
}