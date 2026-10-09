// NVS (Non-Volatile Storage) — енергонезалежне сховище даних
// Збереження даних у Flash-пам’яті за допомогою NVS
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "NVS";

// Struct save
typedef enum
{
	DATA_DS18B20,
	DATA_BME280,
	DATA_RTC
} data_type_t;
typedef struct
{
	data_type_t type;
	float temp;
	float humidity;
	float pressure;
} DeviceConfig;

nvs_handle_t handle;

//------- Save number -----------------------
static esp_err_t save_number(int32_t value)
{
	// 1. Відкриваємо NVS-сховище "storage"
	// NVS_READWRITE — режим читання та запису
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "NVS OPEN error: %s", esp_err_to_name(err));
		return err;
	}

	// 2. Записуємо значення типу int32_t
	// "number" — ключ, за яким значення буде зберігатися в NVS
	err = nvs_set_i32(handle, "number", value);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "NVS SET error: %s", esp_err_to_name(err));

		// Закриваємо NVS-дескриптор перед виходом
		nvs_close(handle);

		return err;
	}

	// 3. Фіксуємо зміни у Flash-пам'яті
	err = nvs_commit(handle);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "NVS COMMIT error: %s", esp_err_to_name(err));

		// Закриваємо NVS-дескриптор перед виходом
		nvs_close(handle);

		return err;
	}

	// 4. Закриваємо NVS-дескриптор
	nvs_close(handle);

	return ESP_OK;
}

//------- Load number -----------------------
static esp_err_t load_number(int32_t *value)
{

	// 1. Відкриваємо NVS-сховище "storage"
	// NVS_READONLY — режим тільки для читання
	esp_err_t err = nvs_open("storage", NVS_READONLY, &handle);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "NVS OPEN error: %s", esp_err_to_name(err));
		return err;
	}

	// 2. Отримуємо значення типу int32_t
	// Значення буде записано за адресою value
	err = nvs_get_i32(handle, "number", value);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "NVS GET error: %s", esp_err_to_name(err));
		// Закриваємо NVS-дескриптор перед виходом
		nvs_close(handle);
		return err;
	}

	// 3. Закриваємо NVS-дескриптор
	nvs_close(handle);

	return ESP_OK;
}

//------- Save structure -----------------------
static esp_err_t save_device_config(const DeviceConfig *config)
{
	// 1. Відкриваємо NVS-сховище "storage"
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "NVS OPEN error: %s", esp_err_to_name(err));
		return err;
	}

	// 2. Записуємо структуру
	err = nvs_set_blob(handle, "device_config", config, sizeof(DeviceConfig));

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "NVS SET BLOB error: %s", esp_err_to_name(err));
		nvs_close(handle);
		return err;
	}

	// 3. Фіксуємо зміни у Flash
	err = nvs_commit(handle);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "NVS COMMIT error: %s", esp_err_to_name(err));
		nvs_close(handle);
		return err;
	}

	// 4. Закриваємо NVS
	nvs_close(handle);

	return ESP_OK;
}

//------- Load structure -----------------------
static esp_err_t load_device_config(DeviceConfig *config)
{
	// 1. Відкриваємо NVS-сховище "storage"
	esp_err_t err = nvs_open("storage", NVS_READONLY, &handle);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "NVS OPEN error: %s", esp_err_to_name(err));
		return err;
	}

	// Розмір структури
	size_t required_size = sizeof(DeviceConfig);

	// 2. Отримуємо структуру з NVS
	err = nvs_get_blob(handle, "device_config", config, &required_size);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "NVS GET BLOB error: %s", esp_err_to_name(err));
		nvs_close(handle);
		return err;
	}

	// 3. Перевіряємо розмір отриманих даних
	if (required_size != sizeof(DeviceConfig))
	{
		ESP_LOGE(TAG, "Invalid structure size: %u", (unsigned int)required_size);
		nvs_close(handle);
		return ESP_ERR_INVALID_SIZE;
	}

	// 4. Закриваємо NVS
	nvs_close(handle);

	return ESP_OK;
}

static const char *data_type_to_string(data_type_t type)
{
	switch (type)
	{
	case DATA_DS18B20:
		return "DATA_DS18B20";
	case DATA_BME280:
		return "DATA_BME280";
	case DATA_RTC:
		return "DATA_RTC";
	default:
		return "UNKNOWN";
	}
}

void app_main(void)
{
	// Дані параметра для збереження
	int32_t num_save = 123;
	int32_t num_load = 0;

	// Дані структури для збереження
	DeviceConfig config_save =
		{
			.type = DATA_BME280,
			.temp = 24.5f,
			.humidity = 58.3f,
			.pressure = 1013.25f};

	DeviceConfig config_load = {0};

	// Init NVS
	esp_err_t err = nvs_flash_init();

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "NVS INIT error: %s", esp_err_to_name(err));
		return;
	}

	//------- NUMBER -----------------------

	// Зберегти
	// err = save_number(num_save);

	// if (err == ESP_OK)
	// {
	// 	ESP_LOGI(TAG, "Number saved successfully");
	// }

	// Зчитати
	err = load_number(&num_load);

	if (err == ESP_OK)
	{
		ESP_LOGI(TAG, "Number loaded: %" PRId32, num_load);
	}

	//------- STRUCTURE -----------------------

	// Зберегти структуру
	// err = save_device_config(&config_save);

	// if (err == ESP_OK)
	// {
	// 	ESP_LOGI(TAG, "DeviceConfig saved successfully");
	// }

	// Зчитати структуру
	err = load_device_config(&config_load);

	if (err == ESP_OK)
	{
		ESP_LOGI(
			TAG,
			"DeviceConfig: type=%s, temperature=%.2f, humidity=%.2f, pressure=%.2f",
			data_type_to_string(config_load.type),
			config_load.temp, config_load.humidity, config_load.pressure);
	}
}