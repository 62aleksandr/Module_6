#pragma once

#include <stdint.h>
#include "time.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

typedef enum
{
	DATA_DS18B20,
	DATA_BME280,
	DATA_RTC
} data_type_t;

typedef struct
{
	float temp;
	float humidity;
	float pressure;
} bme280_data_t;

typedef struct
{
	data_type_t type;
	bme280_data_t bme280_data;
	float temp;
	struct tm time;
} app_data_t;

typedef struct
{
	QueueHandle_t data_queue;
	SemaphoreHandle_t i2c_mutex;
	EventGroupHandle_t sensor_event_group;
} task_context_t;

// Біти завершення задач
#define BME280_DONE_BIT BIT0
#define DS18B20_DONE_BIT BIT1
#define DS1307_DONE_BIT BIT2
#define SSD1306_DONE_BIT BIT3

#define ALL_DONE_BITS (BME280_DONE_BIT | SSD1306_DONE_BIT | DS1307_DONE_BIT | DS18B20_DONE_BIT)