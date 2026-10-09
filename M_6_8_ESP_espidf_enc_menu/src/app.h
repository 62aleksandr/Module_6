#pragma once

#include "encoder.h"
#include "ssd1306.h"

extern ssd1306_t oled_dev;
extern QueueHandle_t encoder_queue;

esp_err_t i2c_init(void);
esp_err_t oled_init(void);
esp_err_t encoder_init(void);
void app_run(rotary_encoder_event_t *event);
