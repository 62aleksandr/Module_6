#pragma once

#include "esp_err.h"

esp_err_t oled_dev_init();
void oled_task(void *pvParameters);