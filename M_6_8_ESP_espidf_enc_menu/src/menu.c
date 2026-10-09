#include <stdint.h>
#include <stdbool.h>
#include "menu.h"

char menu_buf[32]; // Буфер для виведення меню
char temp_buf[32]; // Тимчасовий буфер
menu_item_t *selected_item = NULL;

char bme_data_temp[] = "  25.3 C";
char bme_data_hum[] = "   48.2 %";
char bme_data_pres[] = "  1012.4 hPa";
char rtc_data_data[] = "  16.08.2026";
char rtc_data_time[] = "  21.15.30";

// ------- level 1 menu main ------
menu_item_t main_menu = {
	.name = "Main",
	.type = MENU_SUBMENU,
	.parent = NULL,
	.child = NULL,
	.next = NULL,
	.prev = NULL,
	.data = NULL};

//----level 2 bme rtc -----------
menu_item_t menu_bme = {
	.name = "BME",
	.type = MENU_SUBMENU,
	.parent = &main_menu,
	.child = NULL,
	.next = NULL,
	.prev = NULL,
	.data = NULL};

menu_item_t menu_rtc = {
	.name = "RTC",
	.type = MENU_SUBMENU,
	.parent = &main_menu,
	.child = NULL,
	.next = NULL,
	.prev = &menu_bme,
	.data = NULL};

//----level 3 bme_temp, hum, pres
menu_item_t bme_temp = {
	.name = "TEMP",
	.type = MENU_DATA,
	.parent = &menu_bme,
	.child = NULL,
	.next = NULL,
	.prev = NULL,
	.data = NULL};

menu_item_t bme_hum = {
	.name = "HUM",
	.type = MENU_DATA,
	.parent = &menu_bme,
	.child = NULL,
	.next = NULL,
	.prev = &bme_temp,
	.data = NULL};

menu_item_t bme_pres = {
	.name = "PRES",
	.type = MENU_DATA,
	.parent = &menu_bme,
	.child = NULL,
	.next = NULL, // <-- ТЕПЕР ВПЕРЕД ВЕДЕ НА TEMPERATURE
	.prev = &bme_hum,
	.data = NULL};

//-----level 3 rtc_data, time
menu_item_t rtc_data = {
	.name = "DATA",
	.type = MENU_DATA,
	.parent = &menu_rtc,
	.child = NULL,
	.next = NULL,
	.prev = NULL,
	.data = NULL};

menu_item_t rtc_time = {
	.name = "TIME",
	.type = MENU_DATA,
	.parent = &menu_rtc,
	.child = NULL,
	.next = NULL,
	.prev = &rtc_data,
	.data = NULL};

void menu_init()
{
	//------- init menu---------

	main_menu.child = &menu_bme;
	menu_bme.child = &bme_temp;
	menu_rtc.child = &rtc_data;

	menu_bme.next = &menu_rtc;
	bme_temp.next = &bme_hum;
	bme_hum.next = &bme_pres;
	rtc_data.next = &rtc_time;

	// Початковий вибір — перший пункт меню
	selected_item = &menu_bme;

	bme_temp.data = &bme_data_temp;
	bme_hum.data = &bme_data_hum;
	bme_pres.data = &bme_data_pres;

	rtc_data.data = &rtc_data_data;
	rtc_time.data = &rtc_data_time;

	menu_draw(selected_item);
}

// Відображення меню
void menu_draw(menu_item_t *selected_item)
{
	if (selected_item == NULL)
		return;

	// Очіщння екрану
	ssd1306_clear(&oled_dev);

	menu_item_t *parent = selected_item->parent;
	menu_item_t *item = (parent != NULL) ? parent->child : selected_item;

	int index = 0;

	// Обмеження кількості рядків MENU_MAX_LINES
	while (item != NULL && index < MENU_MAX_LINES)
	{
		const char *marker = (item == selected_item) ? "> " : "  ";

		if (item->type == MENU_SUBMENU)
		{
			snprintf(temp_buf, sizeof(temp_buf), "%s%s", marker, item->name);
		}
		else if (item->type == MENU_DATA)
		{
			// Додано роздільник ": "
			snprintf(temp_buf, sizeof(temp_buf), "%s%s: %s", marker, item->name, (char *)item->data);
		}

		snprintf(menu_buf, sizeof(menu_buf), "%-20s", temp_buf);

		// Відображення рядка
		ssd1306_draw_string(&oled_dev, 0, (3 + index), menu_buf);

		item = item->next;
		index++;
	}
}

// Навігація меню
bool menu_navigate(int8_t diff)
{
	if (selected_item == NULL)
		return false;

	menu_item_t *old_item = selected_item;

	uint8_t index = 0;
	if (diff > 0)
	{
		if (selected_item->next != NULL)
		{
			selected_item = selected_item->next;
		}
		else
		{
			// Опціонально: перехід на початок меню
			while (selected_item->prev != NULL && index < MENU_MAX_LINES)
			{
				selected_item = selected_item->prev;
				index++;
			}
		}
	}
	else if (diff < 0)
	{
		if (selected_item->prev != NULL)
		{
			selected_item = selected_item->prev;
		}
		else
		{

			// Опціонально: перехід у кінець меню
			while (selected_item->next != NULL && index < MENU_MAX_LINES)
			{
				selected_item = selected_item->next;
				index++;
			}
		}
	}

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
