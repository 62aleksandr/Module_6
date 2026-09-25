#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void led_init(void)
{
}

void led_task(void *pvParameters)
{
	QueueHandle_t temp_queue = (QueueHandle_t)pvParameters;

	float received_temp;

	for (;;)
	{
		if (xQueueReceive(temp_queue, &received_temp, pdMS_TO_TICKS(500)) == pdTRUE)
		{
			ESP_LOGI("LED", "Temp: %.2f C", received_temp);
		}
		vTaskDelay(pdMS_TO_TICKS(1000)); // Check every second
	}
}