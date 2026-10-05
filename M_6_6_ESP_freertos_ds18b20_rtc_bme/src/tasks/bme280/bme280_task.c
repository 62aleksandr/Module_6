#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "i2c_bus.h"
#include "app_data.h"

#include "bmp280.h"

static bmp280_t bme_dev;
app_data_t app_data;

// Ініціалізація BMP280
esp_err_t bme280_dev_init()
{
	esp_err_t first_err = ESP_OK;

	memset(&bme_dev, 0, sizeof(bmp280_t));

	esp_err_t err = bmp280_init_desc(&bme_dev,
									 BMP280_I2C_ADDRESS_0,
									 I2C_PORT,
									 I2C_SDA_GPIO,
									 I2C_SCL_GPIO);

	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	bmp280_params_t bmp_params;
	bmp280_init_default_params(&bmp_params);

	err = bmp280_init(&bme_dev, &bmp_params);
	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	return first_err;
}

// Читання даних з BMP280
esp_err_t bme280_read()
{
	esp_err_t err = bmp280_read_float(&bme_dev,
									  &app_data.bme280_data.temp,
									  &app_data.bme280_data.pressure,
									  &app_data.bme280_data.humidity);
	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}

void bme280_task(void *pvParameters)
{
	task_context_t *context = (task_context_t *)pvParameters;

	for (;;)
	{
		// Захватить Mutex перед работой с I2C
		if (xSemaphoreTake(context->i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
		{
			// Прочитать данные из BMP280
			if (bme280_read() == ESP_OK)
			{
				// ESP_LOGI("BME", "%.2f:%.2f:%.2f",
				// 		 app_data.bme280_data.temp,
				// 		 app_data.bme280_data.pressure,
				// 		 app_data.bme280_data.humidity);

				// Указать тип данных
				app_data.type = DATA_BME280;

				// Отправить в Queue
				xQueueSend(context->data_queue, &app_data, pdMS_TO_TICKS(500));
			}
			// Освободить Mutex
			xSemaphoreGive(context->i2c_mutex);
		}

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}