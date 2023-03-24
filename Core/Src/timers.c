/**
  ******************************************************************************
  * @file    timers.c
  * @author  Bernardo López Panadero
  * @brief   Timers management
  *
  ******************************************************************************
 */

#include "stm32f1xx_hal.h"
#include "timers.h"

// Timer handler
extern TIM_HandleTypeDef htim2;
// Array of software timers
static timer_struct timer[TOTAL_TIMERS] = {0};
// Global timestamp in milliseconds
static uint64_t global_time = 0;

/*
 * Check if time setted has
 */
_Bool check_timer(uint8_t id_timer)
{
	_Bool ret = 0;

	if(timer[id_timer].target_time <= global_time){
		timer[id_timer].lock = 0;
		ret = 1;
	}
	return ret;
}

/*
 * Initialize peripheral
 */
void init_timers(void)
{
	HAL_TIM_Base_Start_IT(&htim2);
}

/*
 * This callback is called every millisecond and adds 1 to global timestamp
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	global_time++;
}

/*
 * Set id_timer in milliseconds
 */
_Bool set_timer(uint8_t id_timer, uint32_t time_ms)
{
	_Bool res = 0;

	if(!timer[id_timer].lock){
		timer[id_timer].lock		= 1;
		timer[id_timer].target_time = global_time + time_ms;
		res = 1;
	}

	return res;
}

/*
 * Stop timer
 */
void stop_timer()
{
	HAL_TIM_Base_Stop_IT(&htim2);
}

/*
 * Unlock timer if it is in use to set a new target time
 */
void unlock_timer(uint8_t id_timer)
{
	timer[id_timer].lock = 0;
}


