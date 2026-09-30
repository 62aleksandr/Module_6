#pragma once

// Стан світлофора
typedef enum
{
	STATE_RED = 0,
	STATE_YELLOW,
	STATE_GREEN

} TrafficState_t;

// Подія
typedef enum
{
	EVENT_BUTTON = 0,
	EVENT_1S = 1,
	EVENT_15S = 15,
	EVENT_20S = 20,
	EVENT_40S = 40

} TimerEvent_t;