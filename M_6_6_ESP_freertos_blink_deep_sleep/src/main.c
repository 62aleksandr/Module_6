// Deep Sleep
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "freertos/event_groups.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"

// GPIO LED
#define BLINK_10 GPIO_NUM_10
#define BLINK_11 GPIO_NUM_11
#define BLINK_12 GPIO_NUM_12

// Кнопка пробудження (GPIO 0-21)
#define BUTTON_WAKEUP GPIO_NUM_0

// Ядро для задач
#define xCoreID 1

// Біти завершення задач
#define LED10_DONE_BIT BIT0
#define LED11_DONE_BIT BIT1
#define LED12_DONE_BIT BIT2

#define ALL_DONE_BITS (LED10_DONE_BIT | LED11_DONE_BIT | LED12_DONE_BIT)

// Група подій для синхронізації
static EventGroupHandle_t led_event_group;

// Дескриптори задач
TaskHandle_t led10_task_handle = NULL;
TaskHandle_t led11_task_handle = NULL;
TaskHandle_t led12_task_handle = NULL;

// Час ON/OFF — 1 секунд
#define TIME_MS (1 * 1000)

// Час Deep Sleep — 20 секунд
#define SLEEP_TIME_US (20ULL * 1000000ULL)

static const char *TAG = "MAIN";

// Задача LED10
void led10_task(void *pvParameters)
{
	for (;;)
	{
		gpio_set_level(BLINK_10, 1);
		ESP_LOGI(TAG, "LED 10 ON");
		vTaskDelay(pdMS_TO_TICKS(TIME_MS));

		gpio_set_level(BLINK_10, 0);
		ESP_LOGI(TAG, "LED 10 OFF");
		vTaskDelay(pdMS_TO_TICKS(TIME_MS));

		// Повідомляємо про завершення
		xEventGroupSetBits(led_event_group, LED10_DONE_BIT);
		// Видаляємо поточну задачу
		vTaskDelete(NULL);
	}
}

// Задача LEDD11
void led11_task(void *pvParameters)
{
	gpio_set_level(BLINK_11, 1);
	ESP_LOGI(TAG, "LED 11 ON");
	vTaskDelay(pdMS_TO_TICKS(TIME_MS));

	gpio_set_level(BLINK_11, 0);
	ESP_LOGI(TAG, "LED 11 OFF");
	vTaskDelay(pdMS_TO_TICKS(TIME_MS));

	// Повідомляємо про завершення
	xEventGroupSetBits(led_event_group, LED11_DONE_BIT);
	// Видаляємо поточну задачу
	vTaskDelete(NULL);
}

// Задача LED 12
void led12_task(void *pvParameters)
{
	gpio_set_level(BLINK_12, 1);
	ESP_LOGI(TAG, "LED 12 ON");
	vTaskDelay(pdMS_TO_TICKS(TIME_MS));

	gpio_set_level(BLINK_12, 0);
	ESP_LOGI(TAG, "LED 12 OFF");
	vTaskDelay(pdMS_TO_TICKS(TIME_MS));

	// Повідомляємо про завершення
	xEventGroupSetBits(led_event_group, LED12_DONE_BIT);
	// Видаляємо поточну задачу
	vTaskDelete(NULL);
}

void app_main(void)
{
	// Configure the GPIO pin
	// gpio_reset_pin(BLINK_GPIO);
	// gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

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

	// Налаштовуємо GPIO для LED
	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << BLINK_10) | (1ULL << BLINK_11) | (1ULL << BLINK_12),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE};
	gpio_config(&io_conf);

	// Створюємо групу подій
	led_event_group = xEventGroupCreate();

	// Створюємо задачі LED
	xTaskCreatePinnedToCore(
		led10_task,			// функція задачі
		"LED10_Task",		// назва задачі
		2048,				// розмір стека, байт
		NULL,				// параметри, що передаються задачі
		5,					// пріоритет задачі
		&led10_task_handle, // дескриптор задачі
		xCoreID);			// ядро процесора

	xTaskCreatePinnedToCore(led11_task, "LED11_Task", 2048, NULL, 5, &led11_task_handle, xCoreID);

	xTaskCreatePinnedToCore(led12_task, "LED12_Task", 2048, NULL, 5, &led12_task_handle, xCoreID);

	// Чекаємо завершення всіх трьох задач
	xEventGroupWaitBits(
		led_event_group,
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

	ESP_LOGW(TAG, "Задачі завершені. Deep Sleep - 20 секунд...");

	// Налаштовуємо пробудження через 20 секунд
	esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);

	// Пробудження BUTTON_WAKEUP при LOW
	esp_sleep_enable_ext1_wakeup_io((1ULL << BUTTON_WAKEUP), ESP_EXT1_WAKEUP_ANY_LOW);

	// Залишаємо RTC-периферію увімкненою
	esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

	// Даємо UART час передати лог
	vTaskDelay(pdMS_TO_TICKS(100));

	// Переходимо в Deep Sleep
	esp_deep_sleep_start();
}
