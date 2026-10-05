#pragma once

#include "esp_err.h"

esp_err_t ssd1306_dev_init();
void ssd1306_task(void *pvParameters);