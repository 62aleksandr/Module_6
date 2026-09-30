#pragma once

#include "esp_err.h"
#include "app_data.h"

esp_err_t rtc_dev_init();
void rtc_task(void *pvParameters);