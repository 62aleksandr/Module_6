#include "i2cdev.h"
#include "encoder.h"
#include "ssd1306.h"
#include "string.h"
#include "esp_log.h"

// -----Конфігурація I2C ESP32-S3 ------
const i2c_port_t I2C_PORT = I2C_NUM_0;
const gpio_num_t I2C_SDA_GPIO = GPIO_NUM_16;
const gpio_num_t I2C_SCL_GPIO = GPIO_NUM_15;

//------- Енкодер ------------
const gpio_num_t ENC_A = GPIO_NUM_9;	// CLK
const gpio_num_t ENC_B = GPIO_NUM_10;	// DT
const gpio_num_t ENC_BTN = GPIO_NUM_11; // SW

static const char *TAG = "APP";

QueueHandle_t encoder_queue;
ssd1306_t oled_dev;

// Iніціалізацію i2cdev
esp_err_t i2c_init(void)
{
	// ----- Init i2cdev -----
	esp_err_t err = i2cdev_init();
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "I2C creation failed");
		return err;
	}

	return ESP_OK;
}

// Iніціалізація енкодера

esp_err_t encoder_init()
{
	esp_err_t err;
	// 1. Створення черги
	encoder_queue = xQueueCreate(10, sizeof(rotary_encoder_event_t));
	if (encoder_queue == NULL)
	{
		ESP_LOGE(TAG, "Queue creation failed");
		return ESP_FAIL;
	}

	// 2. Ініціалізація драйвера енкодера
	err = rotary_encoder_init(encoder_queue);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "rotary_encoder_init: %s", esp_err_to_name(err));
		return err;
	}

	// 3. Налаштування структури енкодера
	static rotary_encoder_t encoder = {
		.pin_a = ENC_A,
		.pin_b = ENC_B,
		.pin_btn = ENC_BTN};

	// 4. Додавання енкодера
	err = rotary_encoder_add(&encoder);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "rotary_encoder_add: %s", esp_err_to_name(err));
		return err;
	}

	ESP_LOGI(TAG, "Encoder started");

	return ESP_OK;
}

// Iніціалізація oled
esp_err_t oled_init(void)
{
	memset(&oled_dev, 0, sizeof(ssd1306_t));

	esp_err_t err = ssd1306_init_desc(&oled_dev, I2C_PORT,
									  I2C_SDA_GPIO,
									  I2C_SCL_GPIO);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed init descriptor: %s", esp_err_to_name(err));
		return err;
	}

	// 4. Ініціалізація  OLED
	err = ssd1306_init_display(&oled_dev);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Failed init display: %s", esp_err_to_name(err));
		return err;
	}

	vTaskDelay(pdMS_TO_TICKS(100));

	// Очищення OLED-дисплея
	ssd1306_clear(&oled_dev);
	return ESP_OK;
}
