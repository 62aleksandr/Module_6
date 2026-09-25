#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ssd1306.h"
#include "esp_log.h"
#include <string.h>
#include "i2c_bus.h"

static ssd1306_t oled_dev;

// Ініціалізація OLED-дисплея
esp_err_t oled_dev_init()

{
	memset(&oled_dev, 0, sizeof(ssd1306_t));

	esp_err_t first_err = ESP_OK;

	esp_err_t err = ssd1306_init_desc(&oled_dev,
									  I2C_PORT,
									  I2C_SDA_GPIO,
									  I2C_SCL_GPIO);
	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	err = ssd1306_init_display(&oled_dev);
	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	ssd1306_clear(&oled_dev);

	return first_err;
}

// Оновлення OLED-дисплея
void oled_update(float temp)
{
	char text_buf[32] = {0};
	// Температура
	snprintf(text_buf, sizeof(text_buf), "Temp: %.2f C", temp);
	ssd1306_draw_string(&oled_dev, 5, 2, text_buf);
}

void oled_task(void *pvParameters)
{
	QueueHandle_t temp_queue = (QueueHandle_t)pvParameters;

	float received_temp;

	for (;;)
	{
		if (xQueueReceive(temp_queue, &received_temp, pdMS_TO_TICKS(500)) == pdTRUE)
		{
			ESP_LOGI("OLED", "Temp: %.2f C", received_temp);
			oled_update(received_temp);
		}
		vTaskDelay(pdMS_TO_TICKS(100)); // Check every second
	}
}