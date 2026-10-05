#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_pm.h"

#include "i2c_bus.h"
#include "app_data.h"
#include "ds1307/ds1307_task.h"
#include "ds18b20/ds18b20_task.h"
#include "bme280/bme280_task.h"
#include "ssd1306/ssd1306_task.h"

// Queue parameters
#define QUEUE_LENGTH 5
#define QUEUE_ITEM_SIZE sizeof(app_data_t)

static const char *TAG = "APP";

task_context_t task_context;

void sensors_init(void)
{
	i2c_init();
	// ds18b20_init();
	ds1307_dev_init();
	bme280_dev_init();
	ssd1306_dev_init();
}

void lsleep_init(void)
{
	// Налаштовуємо автоматичне керування живленням
	esp_pm_config_t pm_config = {
		.max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
		.min_freq_mhz = 40,
		.light_sleep_enable = true // ON/OFF true/false
	};
	// Налаштування Power Management
	esp_err_t err = esp_pm_configure(&pm_config);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed to configure power management");
	}
}

void tasks_init(void)
{
	// Create Queue
	task_context.data_queue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);

	if (task_context.data_queue == NULL)
	{
		return;
	}

	// Create Mutex
	task_context.i2c_mutex = xSemaphoreCreateMutex();

	if (task_context.i2c_mutex == NULL)
	{
		return;
	}

	// Create Tasks
	xTaskCreate(ds18b20_task, "ds18b20_task", 4096, &task_context, 5, NULL);
	xTaskCreate(ds1307_task, "ds1307_task", 4096, &task_context, 5, NULL);
	xTaskCreate(bme280_task, "bme280_task", 4096, &task_context, 5, NULL);
	xTaskCreate(ssd1306_task, "ssd1306_task", 4096, &task_context, 4, NULL);
}
