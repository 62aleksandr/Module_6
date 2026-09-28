#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

// Logging tag
static const char *TAG = "QUEUE_PROJECT";

typedef struct
{
	float temp;
	uint32_t timestamp;
} DataPacket_t;

// Queue handle
QueueHandle_t xPtrQueue;

void sender_task(void *pvParameters)
{
	for (;;)
	{
		DataPacket_t packet;

		packet.temp = 25.0f;
		packet.timestamp = xTaskGetTickCount();

		// Відправка структури в чергу
		if (xQueueSend(xPtrQueue, &packet, pdMS_TO_TICKS(100)) == pdPASS)
		{
			ESP_LOGI("TX", "Temp: %.1f, T: %lu", packet.temp, packet.timestamp);
		}
		else
		{
			ESP_LOGW("TX", "Queue full, packet dropped");
		}
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void receiver_task(void *pvParameters)
{
	for (;;)
	{
		DataPacket_t packet;

		if (xQueueReceive(xPtrQueue, &packet, portMAX_DELAY) == pdPASS)
		{
			// Обробка даних
			ESP_LOGI("RX", "Temp: %.1f, T: %lu", packet.temp, packet.timestamp);
			// Log queue status
			ESP_LOGI("RX", "Queue: %u waiting", uxQueueMessagesWaiting(xPtrQueue));
		}
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

void app_main(void)
{
	// Create queue
	xPtrQueue = xQueueCreate(5, sizeof(DataPacket_t));

	if (xPtrQueue == NULL)
	{
		ESP_LOGE(TAG, "Failed to create queue");
		return;
	}
	ESP_LOGI(TAG, "Queue created");

	// Create tasks with increased stack sizes
	xTaskCreate(sender_task, "sender_task", 4096, NULL, 5, NULL);
	xTaskCreate(receiver_task, "receiver_task", 4096, NULL, 4, NULL);
}