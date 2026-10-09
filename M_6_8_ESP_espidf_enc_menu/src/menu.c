#include <stdint.h>
#include <stdbool.h>
#include "menu.h"

char line_buf[32]; // Фіксований розмір масиву для рядка екрану

char bme_data_temp[] = "  25.3 C";
char bme_data_hum[] = "   48.2 %";
char bme_data_pres[] = "  1012.4 hPa";
char rtc_data_data[] = "  16.08.2026";
char rtc_data_time[] = "  21.15.30";

// Передвісники для ієрархії меню
menu_item_t menu_bme, menu_rtc, bme_temp, bme_hum, bme_pres, rtc_data, rtc_time;

// ------- Рівень 1: Головне меню ------
menu_item_t main_menu = {"Main", MENU_SUBMENU, NULL, &menu_bme};

// ------- Рівень 2: Підменю BME та RTC (Зв'язані у кільце) ------
menu_item_t menu_bme = {"BME", MENU_SUBMENU, &main_menu, &bme_temp, &menu_rtc, &menu_rtc};
menu_item_t menu_rtc = {"RTC", MENU_SUBMENU, &main_menu, &rtc_data, &menu_bme, &menu_bme};

// ------- Рівень 3: Дані BME (Зв'язані у кільце) ------
menu_item_t bme_temp = {"TEMP", MENU_DATA, &menu_bme, NULL, &bme_hum, &bme_pres, bme_data_temp};
menu_item_t bme_hum = {"HUM", MENU_DATA, &menu_bme, NULL, &bme_pres, &bme_temp, bme_data_hum};
menu_item_t bme_pres = {"PRES", MENU_DATA, &menu_bme, NULL, &bme_temp, &bme_hum, bme_data_pres};

// ------- Рівень 3: Дані RTC (Зв'язані у кільце) ------
menu_item_t rtc_data = {"DATA", MENU_DATA, &menu_rtc, NULL, &rtc_time, &rtc_time, rtc_data_data};
menu_item_t rtc_time = {"TIME", MENU_DATA, &menu_rtc, NULL, &rtc_data, &rtc_data, rtc_data_time};

menu_item_t *selected_item = &menu_bme;

void menu_init()
{
	menu_draw(selected_item);
}

/// Відображення меню
void menu_draw(menu_item_t *curr_item)
{
	if (!curr_item)
		return;

	ssd1306_clear(&oled_dev);

	// Починаємо обхід з першого елемента на поточному рівні
	menu_item_t *parent = curr_item->parent;
	menu_item_t *item = (parent) ? parent->child : curr_item;
	menu_item_t *start_item = item;

	int index = 0;
	do
	{
		const char *marker = (item == curr_item) ? "> " : "  ";

		// if (item->type == MENU_SUBMENU)
		// 	snprintf(line_buf, sizeof(line_buf), "%s%s", marker, item->name);
		// else
		// 	snprintf(line_buf, sizeof(line_buf), "%s: %s",
		// 			 item->name, (char *)item->data);

		snprintf(line_buf, sizeof(line_buf), (item->type == MENU_SUBMENU) ? "%s%s" : "%s%s: %s",
				 marker, item->name, (char *)item->data);

		// Відображення сформованого рядка
		ssd1306_draw_string(&oled_dev, 0, (3 + index++), line_buf);

		item = item->next;

	} while (item != NULL && item != start_item && index < MENU_MAX_LINES);
}

// Навігація меню (Спрощена завдяки закольцованій структурі)
bool menu_navigate(int8_t diff)
{
	if (!selected_item)
		return false;

	menu_item_t *old_item = selected_item;

	selected_item = (diff > 0) ? selected_item->next : (diff < 0 ? selected_item->prev : selected_item);

	return (old_item != selected_item);
}

// Керування ієрархічним меню
void menu_process_event(rotary_encoder_event_t *event)
{
	if (event == NULL || selected_item == NULL)
		return;

	bool should_redraw = false;

	switch (event->type)
	{
	case RE_ET_CHANGED:
		should_redraw = menu_navigate(event->diff);
		break;

	case RE_ET_BTN_PRESSED:
		if (selected_item->child != NULL)
		{
			selected_item = selected_item->child;
			should_redraw = true;
		}
		break;

	case RE_ET_BTN_LONG_PRESSED:
		if (selected_item->parent != NULL)
		{
			selected_item = selected_item->parent;
			should_redraw = true;
		}
		break;

	default:
		break;
	}

	// Перемальовуємо лише за наявності змін
	if (should_redraw)
	{
		menu_draw(selected_item);
	}
}
