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

static const char *TAG = "SVETOFOR";

QueueHandle_t eventQueue;

// // Стан світлофора
// typedef enum
// {
// 	STATE_RED = 0,
// 	STATE_YELLOW,
// 	STATE_GREEN

// } TrafficState_t;

// // Подія
// typedef enum
// {
// 	EVENT_15S = 0,
// 	EVENT_20S,
// 	EVENT_40S

// } TimerEvent_t;

// Таймер
void timer_callback(void *arg)
{
	static uint32_t counter = 0;
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
}

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

// FSM

void fsm_task(void *pvParameters)
{
	TimerEvent_t event;

	TrafficState_t state = STATE_RED;

	for (;;)
	{
		if (xQueueReceive(eventQueue, &event, portMAX_DELAY))
		{
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
					ESP_LOGI(TAG, "40s -> STATE: RED");
					set_leds(1, 0, 0);
				}
				break;
			}
		}
	}
}

void app_main(void)
{
	// Init GPIO
	leds_init();
	set_leds(1, 0, 0);

	// Queue
	eventQueue = xQueueCreate(5, sizeof(TimerEvent_t));

	if (eventQueue == NULL)
	{
		ESP_LOGE(TAG, "Queue creation failed");
		return;
	}

	// Timer
	const esp_timer_create_args_t timer_args =
		{
			.callback = timer_callback,
			.arg = NULL,
			// .dispatch_method = ESP_TIMER_TASK,
			.name = "fsm_timer"};

	esp_timer_handle_t timer;
	esp_timer_create(&timer_args, &timer);

	// Timer every 1 second
	esp_timer_start_periodic(timer, 1000000);

	// FSM Task
	xTaskCreate(fsm_task, "fsm_task", 4096, NULL, 5, NULL);
}