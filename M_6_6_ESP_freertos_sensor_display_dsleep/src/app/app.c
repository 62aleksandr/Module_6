#include "esp_sleep.h"
#include "i2c_bus.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "app_data.h"
#include "ds1307/ds1307_task.h"
#include "ds18b20/ds18b20_task.h"
#include "bme280/bme280_task.h"
#include "ssd1306/ssd1306_task.h"

// Queue parameters
#define QUEUE_LENGTH 5
#define QUEUE_ITEM_SIZE sizeof(app_data_t)

// Час Deep Sleep — 20 секунд
#define SLEEP_TIME_US (20ULL * 1000000ULL)

// Кнопка пробудження (GPIO 0-21)
#define BUTTON_WAKEUP GPIO_NUM_0

static const char *TAG = "APP";

task_context_t task_context;

void sensors_init(void)
{
	i2c_init();
	ds18b20_init();
	ds1307_dev_init();
	bme280_dev_init();
	ssd1306_dev_init();
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

	task_context.sensor_event_group = xEventGroupCreate();

	// Create Tasks
	xTaskCreate(ds18b20_task, "ds18b20_task", 4096, &task_context, 5, NULL);
	xTaskCreate(ds1307_task, "ds1307_task", 4096, &task_context, 5, NULL);
	xTaskCreate(bme280_task, "bme280_task", 4096, &task_context, 5, NULL);
	xTaskCreate(ssd1306_task, "ssd1306_task", 4096, &task_context, 4, NULL);
}

void dsleep_init(void)
{
	// Перевіряємо причину пробудження
	esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
	if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
	{
		ESP_LOGW(TAG, "Пробудження по ТАЙМЕРУ");
	}
	else if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1)
	{
		ESP_LOGW(TAG, "Пробудження по GPIO (EXT1)");
	}
	else
	{
		ESP_LOGI(TAG, "Первинний запуск системи");
	}

	// Чекаємо завершення всіх трьох задач
	xEventGroupWaitBits(
		task_context.sensor_event_group,
		ALL_DONE_BITS,
		pdTRUE,		  // Скинути біти після зчитування
		pdTRUE,		  // Чекати на ВСІ біти (AND)
		portMAX_DELAY // Чекати до повного завершення
	);

	// Налаштовуємо кнопку як RTC GPIO
	rtc_gpio_init(BUTTON_WAKEUP);
	rtc_gpio_set_direction(BUTTON_WAKEUP, RTC_GPIO_MODE_INPUT_ONLY);

	// Увімкнення внутрішньої підтяжки до 3.3 В
	rtc_gpio_pullup_en(BUTTON_WAKEUP);
	rtc_gpio_pulldown_dis(BUTTON_WAKEUP);

	ESP_LOGW(TAG, "Задачі завершені. Deep Sleep - 60 секунд...");

	// Налаштовуємо пробудження через 20 секунд
	esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);

	// Пробудження BUTTON_WAKEUP при LOW
	esp_sleep_enable_ext1_wakeup_io((1ULL << BUTTON_WAKEUP), ESP_EXT1_WAKEUP_ANY_LOW);

	// Даємо UART час передати лог
	vTaskDelay(pdMS_TO_TICKS(100));

	// Переходимо в Deep Sleep
	esp_deep_sleep_start();
}