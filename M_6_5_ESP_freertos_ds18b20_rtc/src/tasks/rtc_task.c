#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ds1307.h"
#include "esp_log.h"

#include "i2c_bus.h"
#include "app_data.h"

static i2c_dev_t rtc_dev;

static app_data_t app_data;

static void rtc_set_time()
{
	static const char *months[] =
		{
			"Jan", "Feb", "Mar", "Apr",
			"May", "Jun", "Jul", "Aug",
			"Sep", "Oct", "Nov", "Dec"};

	char month[4];
	int year;

	memset(&app_data.time, 0, sizeof(struct tm));

	sscanf(__DATE__, "%3s %d %d",
		   month,
		   &app_data.time.tm_mday,
		   &year);

	app_data.time.tm_year = year - 1900;

	for (int i = 0; i < 12; i++)
	{
		if (strcmp(month, months[i]) == 0)
		{
			app_data.time.tm_mon = i;
			break;
		}
	}

	sscanf(__TIME__, "%d:%d:%d",
		   &app_data.time.tm_hour,
		   &app_data.time.tm_min,
		   &app_data.time.tm_sec);
}

// Ініціалізація RTC
esp_err_t rtc_dev_init()
{
	esp_err_t first_err = ESP_OK;

	memset(&rtc_dev, 0, sizeof(i2c_dev_t));

	esp_err_t err = ds1307_init_desc(&rtc_dev,
									 I2C_PORT,
									 I2C_SDA_GPIO,
									 I2C_SCL_GPIO);

	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	rtc_set_time();

	err = ds1307_set_time(&rtc_dev, &app_data.time);

	if (first_err == ESP_OK && err != ESP_OK)
	{
		first_err = err;
	}

	return first_err;
}

// Читання часу з RTC
esp_err_t rtc_read()
{
	esp_err_t err = ds1307_get_time(&rtc_dev, &app_data.time);

	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}

void rtc_task(void *pvParameters)
{
	task_context_t *context = (task_context_t *)pvParameters;

	for (;;)
	{
		// Захватить Mutex перед работой с I2C
		if (xSemaphoreTake(context->i2c_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
		{
			// Прочитать время из RTC
			if (rtc_read() == ESP_OK)
			{
				ESP_LOGI("RTC", "%02d:%02d:%02d",
						 app_data.time.tm_hour,
						 app_data.time.tm_min,
						 app_data.time.tm_sec);

				// Указать тип данных
				app_data.type = DATA_RTC;

				// Отправить структуру в Queue
				xQueueSend(context->data_queue, &app_data, pdMS_TO_TICKS(500));
			}
			// Освободить Mutex
			xSemaphoreGive(context->i2c_mutex);
		}

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}