// https://components.espressif.com/components/espressif/onewire_bus/versions/1.1.1/readme
// https://components.espressif.com/components/espressif/ds18b20/versions/0.4.0/readme
// pio run -t menuconfig

// SDA_GPIO - 16; SCL_GPIO - 15; SD - 4;

#include "app/app.h"

void app_main(void)
{
	sensors_init();
	tasks_init();
}