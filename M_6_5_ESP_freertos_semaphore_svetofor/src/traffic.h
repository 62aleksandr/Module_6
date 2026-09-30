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
	EVENT_15S = 0,
	EVENT_20S,
	EVENT_40S,
	EVENT_BUTTON

} TimerEvent_t;