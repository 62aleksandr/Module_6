#pragma once

// #include <stdint.h>
// #include <stdbool.h>
#include <stdio.h>
#include "app.h"

// Максимум рядків на екрані
#define MENU_MAX_LINES 4

// Типи пунктів меню
typedef enum
{
	MENU_SUBMENU, // вложенное меню
	MENU_DATA,	  // отображение/изменение значения
	MENU_ACTION	  // выполнение действия
} menu_type_t;

// Структура пункту меню
typedef struct menu_item_t
{
	const char *name; // Назва пункту меню
	menu_type_t type; // Тип пункту:

	struct menu_item_t *parent; // перехід на верхній рівень
	struct menu_item_t *child;	// перехід на нижній рівень
	struct menu_item_t *next;	// Наступний пункт на цьому ж рівні вниз
	struct menu_item_t *prev;	// Наступний пункт на цьому ж рівні вгору
	void *data;

} menu_item_t;

// Глобальний вказівник на поточний вибраний пункт
extern menu_item_t *selected_item;

// Прототипи функцій
void menu_init(void);
void menu_draw(menu_item_t *selected_item);
bool menu_navigate(int8_t diff);
void menu_process_event(rotary_encoder_event_t *event);