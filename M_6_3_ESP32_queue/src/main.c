#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

// Queue parameters
#define QUEUE_LENGTH 5
#define QUEUE_ITEM_SIZE sizeof(float)

// Logging tag
static const char *TAG = "QUEUE_PROJECT";

// Queue handle
QueueHandle_t sensorQueue;

// Task to simulate sensor data
void sensor_task(void *pvParameters)
{
	float sensor_data = 25.0f; // Mock temperature
	for (;;)
	{
		// Send data to queue
		if (xQueueSend(sensorQueue, &sensor_data, pdMS_TO_TICKS(1000)) == pdTRUE)
		{
			ESP_LOGI(TAG, "Sent sensor: %.1f", sensor_data);
		}
		else
		{
			ESP_LOGE(TAG, "Failed to send sensor: %.1f", sensor_data);
		}
		sensor_data += 0.5f;			 // Simulate temperature change
		vTaskDelay(pdMS_TO_TICKS(2000)); // Send every 2 seconds
	}
}

// Task to simulate high-priority alerts
void alert_task(void *pvParameters)
{
	float alert_data = 99.9f; // High-priority alert data
	for (;;)
	{
		// Send data to queue
		if (xQueueSend(sensorQueue, &alert_data, pdMS_TO_TICKS(1000)) == pdTRUE)
		{
			ESP_LOGI(TAG, "Sent alert: %.1f", alert_data);
		}
		else
		{
			ESP_LOGE(TAG, "Failed to send alert: %.1f", alert_data);
		}
		alert_data += 0.1f;				 // Increment for next alert
		vTaskDelay(pdMS_TO_TICKS(5000)); // Send every 5 seconds
	}
}

// Task to process queue data
void processor_task(void *pvParameters)
{
	float received_data;
	for (;;)
	{
		// Receive data from queue
		if (xQueueReceive(sensorQueue, &received_data, pdMS_TO_TICKS(6000)) == pdTRUE)
		{
			const char *type = received_data >= 99.9f ? "Alert" : "Sensor";
			ESP_LOGI(TAG, "Processed: %.1f (%s)", received_data, type);
		}
		else
		{
			ESP_LOGE(TAG, "Receive timeout");
		}

		// Log queue status
		ESP_LOGI(TAG, "Queue: %u waiting", uxQueueMessagesWaiting(sensorQueue));

		vTaskDelay(pdMS_TO_TICKS(1000)); // Check every second
	}
}

void app_main(void)
{
	// Create queue
	sensorQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
	if (sensorQueue == NULL)
	{
		ESP_LOGE(TAG, "Failed to create queue");
		return;
	}
	ESP_LOGI(TAG, "Queue created");

	// Create tasks with increased stack sizes
	xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
	xTaskCreate(alert_task, "alert_task", 4096, NULL, 6, NULL); // Higher priority
	xTaskCreate(processor_task, "processor_task", 4096, NULL, 5, NULL);
}