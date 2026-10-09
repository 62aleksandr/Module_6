#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

#define CORE_ID 1
#define TASK_STACK_SIZE 4096

static const char *TAG = "MAIN";

// Дескриптори задач
TaskHandle_t task1_handle = NULL;
TaskHandle_t task2_handle = NULL;
TaskHandle_t task3_handle = NULL;
TaskHandle_t task4_handle = NULL;
TaskHandle_t task5_handle = NULL;
TaskHandle_t task6_handle = NULL;
TaskHandle_t monitor_handle = NULL;

// Задача, що хаотично працює з пам'яттю
void memory_chaos_task(void *pvParameters)
{
	int task_id = (int)pvParameters;

	for (;;)
	{
		// 1. ВИДІЛЯЄМО ВЕЛИКИЙ БЛОК: від 8 КБ (8192) до 32 КБ (32768)
		size_t large_size = 8192 + (rand() % (32768 - 8192 + 1));
		void *large_buffer = heap_caps_malloc(large_size, MALLOC_CAP_INTERNAL);

		// 2. ВИДІЛЯЄМО МАЛИЙ БЛОК: від 64 до 256 байт
		size_t small_size = 64 + (rand() % (256 - 64 + 1));
		void *small_buffer = heap_caps_malloc(small_size, MALLOC_CAP_INTERNAL);

		if (large_buffer != NULL && small_buffer != NULL)
		{
			// ESP_LOGI(TAG, "Task %d: Allocated pair (%d КБ + %d байт)", task_id, (int)(large_size / 1024), (int)small_size);

			// Випадкова затримка утримання обох блоків
			vTaskDelay(pdMS_TO_TICKS(10 + (rand() % 50)));

			// 3. СТВОРЮЄМО ХАОС: з імовірністю 50% звільняємо спочатку малий або великий блок
			if (rand() % 2 == 0)
			{
				heap_caps_free(large_buffer);
				vTaskDelay(pdMS_TO_TICKS(5 + (rand() % 15))); // Пауза між звільненнями
				heap_caps_free(small_buffer);
			}
			else
			{
				heap_caps_free(small_buffer);
				vTaskDelay(pdMS_TO_TICKS(5 + (rand() % 15)));
				heap_caps_free(large_buffer);
			}
		}
		else
		{
			// Якщо щось не виділилось — чистимо те, що встигло зайняти пам'ять
			if (large_buffer)
				heap_caps_free(large_buffer);
			if (small_buffer)
				heap_caps_free(small_buffer);

			ESP_LOGE(TAG, "Task %d: MALLOC FAILED! Memory is heavily fragmented!", task_id);
		}

		// Коротка пауза перед наступним колом
		vTaskDelay(pdMS_TO_TICKS(1));
		// taskYIELD();
	}
}

// Heap Monitor Task
void heap_monitor_task(void *pvParameters)
{
	for (;;)
	{
		// Пауза на початку
		vTaskDelay(pdMS_TO_TICKS(2000));

		ESP_LOGI(TAG, "========== HEAP MONITOR ==========");
		uint32_t free_heap = esp_get_free_heap_size();
		uint32_t max_block = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);

		ESP_LOGI(TAG, "Total Free heap : %u bytes", (unsigned)free_heap);
		ESP_LOGI(TAG, "Largest Block   : %u bytes", (unsigned)max_block);

		// Розрахунок фрагментації у відсотках
		float frag_ratio = (1.0f - ((float)max_block / (float)free_heap)) * 100.0f;
		ESP_LOGI(TAG, "Fragmentation   : %.2f %%", frag_ratio);
		ESP_LOGI(TAG, "==================================");
	}
}

// app_main
void app_main(void)
{
	srand(12345); // Ініціалізація генератора випадкових чисел

	// 1. Динамічні задачі (Передаємо ID задачі як параметр)
	xTaskCreatePinnedToCore(memory_chaos_task, "Chaos_1", TASK_STACK_SIZE, (void *)1, 1, &task1_handle, CORE_ID);
	xTaskCreatePinnedToCore(memory_chaos_task, "Chaos_2", TASK_STACK_SIZE, (void *)2, 1, &task2_handle, CORE_ID);
	xTaskCreatePinnedToCore(memory_chaos_task, "Chaos_3", TASK_STACK_SIZE, (void *)3, 2, &task3_handle, CORE_ID);
	xTaskCreatePinnedToCore(memory_chaos_task, "Chaos_4", TASK_STACK_SIZE, (void *)4, 2, &task4_handle, CORE_ID);
	xTaskCreatePinnedToCore(memory_chaos_task, "Chaos_5", TASK_STACK_SIZE, (void *)5, 3, &task5_handle, CORE_ID);

	// 2. Статична задача
	static StaticTask_t task6_tcb;
	static StackType_t task6_stack[TASK_STACK_SIZE];
	task6_handle = xTaskCreateStaticPinnedToCore(
		memory_chaos_task, "Chaos_6_Static", TASK_STACK_SIZE, (void *)6, 3, task6_stack, &task6_tcb, CORE_ID);

	// 3. Монітор пам'яті
	xTaskCreatePinnedToCore(heap_monitor_task, "HeapMonitor", TASK_STACK_SIZE, NULL, 4, &monitor_handle, CORE_ID);
}