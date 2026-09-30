#pragma once

#include <time.h>

typedef enum
{
	DATA_TEMP,
	DATA_RTC
} data_type_t;

typedef struct
{
	data_type_t type;
	float temp;
	struct tm time;
} app_data_t;

typedef struct
{
	QueueHandle_t data_queue;
	SemaphoreHandle_t i2c_mutex;
} task_context_t;