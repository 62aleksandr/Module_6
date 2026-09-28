#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

// Queue parameters
#define QUEUE_LENGTH 5
#define QUEUE_ITEM_SIZE sizeof(float)

// BOOT button on ESP32-S3
#define BOOT_GPIO GPIO_NUM_0

// Logging tag
static const char *TAG = "QUEUE";

// Queue handle
QueueHandle_t sensorQueue;

// GPIO ISR
void IRAM_ATTR gpio_isr_handler(void *arg)
{
	float data = 15.0f;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xQueueSendFromISR(sensorQueue, &data, &xHigherPriorityTaskWoken);

	if (xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR();
	}
}

// Task: simulate sensor data
void sensor_task(void *pvParameters)
{
	float sensor_data = 25.0f; // Mock temperature

	for (;;)
	{
		// Send data to queue
		if (xQueueSend(sensorQueue, &sensor_data, pdMS_TO_TICKS(1000)) == pdTRUE)
		{
			ESP_LOGI(TAG, "Sensor -> Queue: %.1f", sensor_data);
		}
		else
		{
			ESP_LOGE(TAG, "Failed to send sensor: %.1f", sensor_data);
		}

		sensor_data += 0.01f;			 // Increment for next sensor reading
		vTaskDelay(pdMS_TO_TICKS(1000)); // Send every 1 seconds
	}
}

// Task: process queue data
void processor_task(void *pvParameters)
{
	float received_data;

	for (;;)
	{
		// Receive data from queue
		if (xQueueReceive(sensorQueue, &received_data, pdMS_TO_TICKS(1000)) == pdTRUE)
		{
			if (received_data == 15.0f)
			{
				ESP_LOGI(TAG, "ISR -> Queue -> Processor: %.1f", received_data);
			}
			else
			{
				ESP_LOGI(TAG, "Sensor -> Queue -> Processor: %.1f", received_data);
			}
		}
		else
		{
			ESP_LOGE(TAG, "Receive timeout");
		}

		// Log queue status
		ESP_LOGI(TAG, "Queue: %u waiting", uxQueueMessagesWaiting(sensorQueue));

		vTaskDelay(pdMS_TO_TICKS(200)); // Delay 200 ms
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

	// Configure BOOT button GPIO0
	gpio_config_t io_conf = {};
	io_conf.pin_bit_mask = 1ULL << BOOT_GPIO;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	gpio_config(&io_conf);

	// Install ISR service
	gpio_install_isr_service(0);

	// Attach ISR to BOOT button
	gpio_isr_handler_add(BOOT_GPIO, gpio_isr_handler, NULL);
	// gpio_isr_handler_add(BOOT_GPIO, gpio_isr_handler, (void *)15);

	// Create tasks
	xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
	xTaskCreate(processor_task, "processor_task", 4096, NULL, 4, NULL);
}