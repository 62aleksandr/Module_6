#include "i2cdev.h"

// Конфігурація I2C ESP32-S3
const i2c_port_t I2C_PORT = I2C_NUM_0;
const gpio_num_t I2C_SDA_GPIO = GPIO_NUM_16;
const gpio_num_t I2C_SCL_GPIO = GPIO_NUM_15;

// Iніціалізацію бібліотеки i2cdev
esp_err_t i2c_init(void)
{
	// ----- Init i2cdev -----
	esp_err_t err = i2cdev_init();
	if (err != ESP_OK)
	{
		return err;
	}

	return ESP_OK;
}