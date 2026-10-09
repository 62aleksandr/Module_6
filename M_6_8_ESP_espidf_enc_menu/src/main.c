#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "menu.h"

void app_main(void)
{
	rotary_encoder_event_t event;

	i2c_init();
	oled_init();
	encoder_init();
	menu_init();

	while (1)
	{
		// Обробка подій енкодера
		if (xQueueReceive(encoder_queue, &event, 0) == pdTRUE)
		{
			menu_process_event(&event);
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
