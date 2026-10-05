#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#include "esp_timer.h"
#include "esp_log.h"
#include "traffic.h"

#define LED_RED GPIO_NUM_10
#define LED_YELLOW GPIO_NUM_11
#define LED_GREEN GPIO_NUM_12

#define BUTTON_0 GPIO_NUM_0

static uint32_t counter = 0;

static const char *TAG = "SVETOFOR";

static esp_timer_handle_t timer;
static SemaphoreHandle_t counterMutex;
static QueueHandle_t eventQueue;
static SemaphoreHandle_t buttonSemaphore;

void leds_init()
{
	gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);
	gpio_set_direction(LED_YELLOW, GPIO_MODE_OUTPUT);
	gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
}

void set_leds(uint32_t red, uint32_t yellow, uint32_t green)
{
	gpio_set_level(LED_RED, red);
	gpio_set_level(LED_YELLOW, yellow);
	gpio_set_level(LED_GREEN, green);
}

// Callback BUTTON_0
static void IRAM_ATTR button_isr_handler(void *arg)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(buttonSemaphore, &xHigherPriorityTaskWoken);

	if (xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR();
	}
}

// Callback timer
void timer_callback(void *arg)
{

	if (xSemaphoreTake(counterMutex, pdMS_TO_TICKS(100)) == pdTRUE)
	{
		counter++;
		TimerEvent_t event;

		switch (counter)
		{
		case 15:
			event = EVENT_15S;
			xQueueSend(eventQueue, &event, 0);
			break;

		case 20:
			event = EVENT_20S;
			xQueueSend(eventQueue, &event, 0);
			break;

		case 40:
			event = EVENT_40S;
			if (xQueueSend(eventQueue, &event, 0) == pdPASS)
			{
				counter = 0;
			}
			break;
		}
		xSemaphoreGive(counterMutex);
	}
}

// Task FSM
void fsm_task(void *pvParameters)
{
	TimerEvent_t event;

	TrafficState_t state = STATE_RED;

	for (;;)
	{
		if (xQueueReceive(eventQueue, &event, pdMS_TO_TICKS(500)) == pdTRUE)
		{
			if (event == EVENT_BUTTON)
			{
				if (xSemaphoreTake(counterMutex, pdMS_TO_TICKS(100)) == pdTRUE)
				{
					counter = 0;
					xSemaphoreGive(counterMutex);
				}

				state = STATE_RED;
				ESP_LOGI(TAG, "EVENT_BUTTON: RED");
				set_leds(1, 0, 0);
				continue;
			}

			switch (state)
			{
			case STATE_RED:

				if (event == EVENT_15S)
				{
					state = STATE_YELLOW;
					ESP_LOGI(TAG, "15s -> STATE: YELLOW");
					set_leds(0, 1, 0);
				}
				break;

			case STATE_YELLOW:

				if (event == EVENT_20S)
				{
					state = STATE_GREEN;
					ESP_LOGI(TAG, "20s -> STATE: GREEN");
					set_leds(0, 0, 1);
				}
				break;

			case STATE_GREEN:

				if (event == EVENT_40S)
				{
					state = STATE_RED;
					ESP_LOGI(TAG, "40/0s -> STATE: RED");
					set_leds(1, 0, 0);
				}
				break;
			}
		}
	}
}

// Task Button
void button_task(void *pvParameters)
{
	TimerEvent_t event = EVENT_BUTTON;

	for (;;)
	{
		// Очікуємо сигнал від ISR
		if (xSemaphoreTake(buttonSemaphore, pdMS_TO_TICKS(100)) == pdTRUE)
		{
			// Debounce при натисканні
			vTaskDelay(pdMS_TO_TICKS(50));

			// Перевірка стану кнопки (захист від завад)
			if (gpio_get_level(BUTTON_0) == 0)
			{
				// Формуємо подію та передаємо її до FSM
				if (xQueueSend(eventQueue, &event, 0) != pdPASS)
				{
					ESP_LOGW(TAG, "Event queue full");
				}

				// Чекаємо відпускання кнопки
				while (gpio_get_level(BUTTON_0) == 0)
				{
					vTaskDelay(pdMS_TO_TICKS(10));
				}

				// Debounce при ВІДПУСКАННІ (чекаємо, поки затихнуть контакти)
				vTaskDelay(pdMS_TO_TICKS(50));
			}

			// КРИТИЧНО ДЛЯ ПРОМИСЛОВОГО КОДУ:
			// Очищаємо семафор від усього сміття, яке набігло в ISR
			// під час дребезгу як при натисканні, так і при відпусканні.
			xSemaphoreTake(buttonSemaphore, 0);
		}
	}
}

void app_main(void)
{
	// Create Mutex
	counterMutex = xSemaphoreCreateMutex();

	if (counterMutex == NULL)
	{
		ESP_LOGE(TAG, "Mutex creation failed");
		return;
	}

	// Create Queue
	eventQueue = xQueueCreate(5, sizeof(TimerEvent_t));

	if (eventQueue == NULL)
	{
		ESP_LOGE(TAG, "Queue creation failed");
		return;
	}

	// Create Semaphore
	buttonSemaphore = xSemaphoreCreateBinary();

	if (buttonSemaphore == NULL)
	{
		ESP_LOGE(TAG, "Semaphore creation failed");
		return;
	}

	// Init GPIO
	leds_init();
	set_leds(1, 0, 0);

	// Timer
	const esp_timer_create_args_t timer_args =
		{
			.callback = timer_callback,
			.arg = NULL,
			// .dispatch_method = ESP_TIMER_TASK,
			.name = "fsm_timer"};

	esp_timer_create(&timer_args, &timer);

	// Timer every 1 second
	esp_timer_start_periodic(timer, 1000000);

	// Configure BOOT button GPIO0
	gpio_config_t io_conf = {};
	io_conf.pin_bit_mask = 1ULL << BUTTON_0;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	gpio_config(&io_conf);

	// Install ISR service
	gpio_install_isr_service(0);

	// Attach ISR to BOOT button
	gpio_isr_handler_add(BUTTON_0, button_isr_handler, NULL);
	// gpio_isr_handler_add(BOOT_GPIO, gpio_isr_handler, (void *)15);

	// Task Button
	xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);

	// Task FSM
	xTaskCreate(fsm_task, "fsm_task", 4096, NULL, 5, NULL);
}