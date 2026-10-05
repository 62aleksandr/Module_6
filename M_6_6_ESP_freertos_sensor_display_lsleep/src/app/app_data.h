#pragma once

#include <time.h>

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
} task_context_t;

#define SLEEP_TIME_MS (60 * 1000)