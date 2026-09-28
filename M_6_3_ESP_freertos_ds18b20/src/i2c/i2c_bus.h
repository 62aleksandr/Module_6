#pragma once

#include "driver/i2c.h"
#include "driver/gpio.h"

extern const i2c_port_t I2C_PORT;
extern const gpio_num_t I2C_SDA_GPIO;
extern const gpio_num_t I2C_SCL_GPIO;

esp_err_t i2c_init(void);