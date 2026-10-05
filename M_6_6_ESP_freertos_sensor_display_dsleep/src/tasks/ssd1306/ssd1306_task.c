#include "ssd1306.h"
#include "esp_log.h"
#include <string.h>
#include "i2c_bus.h"
#include "app_data.h"

static ssd1306_t oled_dev;

// Ініціалізація OLED-дисплея
esp_err_t ssd1306_dev_init()
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
void ssd1306_update(app_data_t *app_data)
{
	char text_buf[32] = {0};

	if (app_data->type == DATA_RTC)
	{
		// Дата
		snprintf(text_buf, sizeof(text_buf),
				 "%02d.%02d.%04d",
				 app_data->time.tm_mday,
				 app_data->time.tm_mon + 1,
				 app_data->time.tm_year + 1900);

		ssd1306_draw_string(&oled_dev, 5, 0, text_buf);

		// Час
		snprintf(text_buf, sizeof(text_buf),
				 "%02d:%02d:%02d",
				 app_data->time.tm_hour,
				 app_data->time.tm_min,
				 app_data->time.tm_sec);

		ssd1306_draw_string(&oled_dev, 5, 1, text_buf);

		ESP_LOGI("OLED", "RTC: %02d:%02d:%02d",
				 app_data->time.tm_hour,
				 app_data->time.tm_min,
				 app_data->time.tm_sec);
	}

	if (app_data->type == DATA_DS18B20)
	{
		// Температура
		snprintf(text_buf, sizeof(text_buf), "Temp: %.2f C", app_data->temp);

		ssd1306_draw_string(&oled_dev, 5, 3, text_buf);

		ESP_LOGI("OLED", "ds18b20: %.2f C", app_data->temp);
	}

	if (app_data->type == DATA_BME280)
	{
		// Температура
		snprintf(text_buf, sizeof(text_buf), "Temp: %.2f C", app_data->bme280_data.temp);
		ssd1306_draw_string(&oled_dev, 5, 5, text_buf);

		// Вологість
		snprintf(text_buf, sizeof(text_buf), "Hum: %.2f %%", app_data->bme280_data.humidity);
		ssd1306_draw_string(&oled_dev, 5, 6, text_buf);

		// Тиск
		snprintf(text_buf, sizeof(text_buf), "Pres: %.2f hPa", app_data->bme280_data.pressure);
		ssd1306_draw_string(&oled_dev, 5, 7, text_buf);

		ESP_LOGI("OLED", "BME280: Temp: %.2f C, Hum: %.2f %%, Pres: %.2f hPa",
				 app_data->bme280_data.temp,
				 app_data->bme280_data.humidity,
				 app_data->bme280_data.pressure);
	}
}

void ssd1306_task(void *pvParameters)
{
	task_context_t *context = (task_context_t *)pvParameters;

	app_data_t app_data;

	while (xQueueReceive(context->data_queue, &app_data, pdMS_TO_TICKS(1000)) == pdTRUE)

	{
		// Захопити Mutex перед роботою з I2C
		if (xSemaphoreTake(context->i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
		{
			ssd1306_update(&app_data);
			// Звільнити Mutex
			xSemaphoreGive(context->i2c_mutex);
		}
	}

	vTaskDelay(pdMS_TO_TICKS(100));

	// Повідомляємо про завершення
	xEventGroupSetBits(context->sensor_event_group, SSD1306_DONE_BIT);
	// Видаляємо поточну задачу
	vTaskDelete(NULL);
}
