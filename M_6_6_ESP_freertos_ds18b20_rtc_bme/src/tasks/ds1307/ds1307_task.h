#pragma once

#include "esp_err.h"
#include "app_data.h"

esp_err_t ds1307_dev_init();
void ds1307_task(void *pvParameters);