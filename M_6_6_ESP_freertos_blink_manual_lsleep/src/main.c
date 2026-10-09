// Manual Light Sleep
#include <stdio.h>
#include "esp_sleep.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_pm.h"

#define BLINK_10 GPIO_NUM_10
#define BLINK_11 GPIO_NUM_11
#define BLINK_12 GPIO_NUM_12

#define xCoreID 1
#define SLEEP_TIME_US (20ULL * 1000000ULL) // 20 секунд

static const char *TAG = "MANUAL_SLEEP";

// Хендли задач
TaskHandle_t led10_task_handle = NULL;
TaskHandle_t led11_task_handle = NULL;
TaskHandle_t led12_task_handle = NULL;
TaskHandle_t control_task_handle = NULL;

static bool global_led_state = false;

// Задача LED10
void led10_task(void *pvParameters)
{
	for (;;)
	{
		// Чекаємо дозволу від головної задачі
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		gpio_set_level(BLINK_10, global_led_state);
		ESP_LOGI(TAG, "LED 10: %s", global_led_state ? "ON" : "OFF");

		// Звітуємо головній задачі, що ми закінчили
		xTaskNotifyGive(control_task_handle);
	}
}

// Задача LED11
void led11_task(void *pvParameters)
{
	for (;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		gpio_set_level(BLINK_11, global_led_state);
		ESP_LOGI(TAG, "LED 11: %s", global_led_state ? "ON" : "OFF");

		xTaskNotifyGive(control_task_handle);
	}
}

// Задача LED12
void led12_task(void *pvParameters)
{
	for (;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		gpio_set_level(BLINK_12, global_led_state);
		ESP_LOGI(TAG, "LED 12: %s", global_led_state ? "ON" : "OFF");

		xTaskNotifyGive(control_task_handle);
	}
}

// Головна керуюча задача
void main_control_task(void *pvParameters)
{
	for (;;)
	{
		global_led_state = !global_led_state;

		// 1. Штовхаємо всі три LED-задачі в роботу
		xTaskNotifyGive(led10_task_handle);
		xTaskNotifyGive(led11_task_handle);
		xTaskNotifyGive(led12_task_handle);

		// 2. Чекаємо відповіді від усіх трьох задач
		for (int i = 0; i < 3; i++)
		{
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		}

		ESP_LOGI(TAG, "All LED tasks completed. Entering Light Sleep...");

		// 3. Задаємо таймер пробудження
		esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);

		// 4. Очищаємо UART буфер
		uart_wait_tx_idle_polling(CONFIG_ESP_CONSOLE_UART_NUM);

		// 5. Ручний запуск сну
		esp_light_sleep_start();

		// Код продовжиться ТУТ рівно через 20 секунд
		esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
		ESP_LOGI(TAG, "Wakeup cause: %d", cause);
		ESP_LOGI(TAG, "Wake up from Light Sleep finished.");

		// Даємо FreeRTOS 50 мс на синхронізацію внутрішніх таймерів після сну
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

void app_main(void)
{
	// Налаштовуємо GPIO LED
	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << BLINK_10) | (1ULL << BLINK_11) | (1ULL << BLINK_12),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE};
	gpio_config(&io_conf);

	// КРИТИЧНО ДЛЯ LIGHT SLEEP: ініціалізація годинника PM
	// Без цього макросу або структури функція esp_light_sleep_start() ламає таймери FreeRTOS
	esp_pm_config_t pm_config = {
		.max_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ,
		.min_freq_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ, // не знижуємо частоту процесора, лише фіксуємо RTC
		.light_sleep_enable = false						 // Автоматичний сон ВИМКНЕНО (ми спимо вручну!)
	};
	esp_pm_configure(&pm_config);

	// Створюємо керуючу задачу ПЕРШОЮ (щоб отримати її дескриптор)
	xTaskCreatePinnedToCore(main_control_task, "Control_Task", 3584, NULL, 6, &control_task_handle, xCoreID);

	// Створюємо задачі LED
	xTaskCreatePinnedToCore(led10_task, "LED10_Task", 2048, NULL, 5, &led10_task_handle, xCoreID);
	xTaskCreatePinnedToCore(led11_task, "LED11_Task", 2048, NULL, 5, &led11_task_handle, xCoreID);
	xTaskCreatePinnedToCore(led12_task, "LED12_Task", 2048, NULL, 5, &led12_task_handle, xCoreID);
}