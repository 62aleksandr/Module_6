#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

// Define the GPIO pin for the LED
const gpio_num_t BLINK_15 = GPIO_NUM_15;
const gpio_num_t BLINK_16 = GPIO_NUM_16;
const gpio_num_t BLINK_17 = GPIO_NUM_17;

extern "C" void app_main(void)
{
	// Configure the GPIO pin
	// gpio_reset_pin(BLINK_GPIO);
	// gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << BLINK_15) | (1ULL << BLINK_16) | (1ULL << BLINK_17),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE};
	gpio_config(&io_conf);

	// Blink loop
	while (1)
	{
		// LED_15 ON
		gpio_set_level(BLINK_15, 1);
		vTaskDelay(500 / portTICK_PERIOD_MS); // Delay 500 ms

		// LED_15 OFF LED_16 ON
		gpio_set_level(BLINK_15, 0);
		gpio_set_level(BLINK_16, 1);
		vTaskDelay(300 / portTICK_PERIOD_MS); // Delay 300 ms

		// LED_16 OFF LED_17 ON
		gpio_set_level(BLINK_16, 0);
		gpio_set_level(BLINK_17, 1);
		vTaskDelay(300 / portTICK_PERIOD_MS); // Delay 300 ms

		// LED_17 OFF
		gpio_set_level(BLINK_17, 0);
		vTaskDelay(100 / portTICK_PERIOD_MS); // Delay 100 ms
	}
}
