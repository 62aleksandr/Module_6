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
	uint8_t *buffer;
	size_t length;
	uint32_t timestamp;
} DataPacket_t;

// Queue handle
QueueHandle_t xPtrQueue;

void sender_task(void *pvParameters)
{
	for (;;)
	{
		// Безпечне виділення пам'яті
		DataPacket_t *packet = malloc(sizeof(DataPacket_t));
		if (packet == NULL)
		{
			ESP_LOGE("TX", "Failed to allocate packet struct");
			vTaskDelay(pdMS_TO_TICKS(100));
			continue;
		}

		packet->buffer = malloc(256);
		if (packet->buffer == NULL)
		{
			ESP_LOGE("TX", "Failed to allocate buffer");
			free(packet); // Звільняємо структуру, якщо буфер не створився
			vTaskDelay(pdMS_TO_TICKS(100));
			continue;
		}

		// Заповнення даними (наприклад, нулями або корисним навантаженням)
		memset(packet->buffer, 0, 256);
		packet->length = 256;
		packet->timestamp = xTaskGetTickCount();

		// Відправка вказівника в чергу
		if (xQueueSend(xPtrQueue, &packet, pdMS_TO_TICKS(100)) != pdPASS)
		{
			// Черга повна — очищаємо все
			free(packet->buffer);
			free(packet);
			ESP_LOGW("TX", "Queue full, packet dropped");
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void receiver_task(void *pvParameters)
{
	DataPacket_t *packet;
	for (;;)
	{

		if (xQueueReceive(xPtrQueue, &packet, portMAX_DELAY) == pdPASS)
		{
			// Обробка даних
			ESP_LOGI("RX", "Len: %u, T: %lu", packet->length, packet->timestamp);
			// Log queue status
			ESP_LOGI("RX", "Queue: %u waiting", uxQueueMessagesWaiting(xPtrQueue));

			// Отримувач звільняє пам'ять після обробки
			free(packet->buffer);
			free(packet);
		}
	}
}

void app_main(void)
{
	// Create queue
	xPtrQueue = xQueueCreate(5, sizeof(DataPacket_t *));

	if (xPtrQueue == NULL)
	{
		ESP_LOGE(TAG, "Failed to create queue");
		return;
	}
	ESP_LOGI(TAG, "Queue created");

	// Create tasks with increased stack sizes
	xTaskCreate(sender_task, "sender_task", 4096, NULL, 5, NULL);
	xTaskCreate(receiver_task, "receiver_task", 4096, NULL, 5, NULL);
}