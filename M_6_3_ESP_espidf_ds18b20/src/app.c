#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "temp_task.h"
#include "oled_task.h"
#include "i2c_bus.h"

// Queue parameters
#define QUEUE_LENGTH 5
#define QUEUE_ITEM_SIZE sizeof(float)

// Queue handle
QueueHandle_t temp_queue;

void sensors_init(void)
{
	i2c_init();
	ds18b20_init();
	oled_dev_init();
}

void tasks_init(void)
{
	temp_queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);

	if (temp_queue == NULL)
	{
		return;
	}

	xTaskCreate(temp_task, "temp_task", 4096, temp_queue, 5, NULL);
	xTaskCreate(oled_task, "oled_task", 4096, temp_queue, 4, NULL);
}