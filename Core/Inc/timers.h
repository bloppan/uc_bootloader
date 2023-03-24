/**
  ******************************************************************************
  * @file    timers.h
  * @author  Bernardo López Panadero
  * @brief   Definiciones y funciones de los timers
  *
  ******************************************************************************
 */

#ifndef INC_TIMERS_H_
#define INC_TIMERS_H_

#include "stm32f1xx_hal.h"

#define	TOTAL_TIMERS	30	// Total number of software timers avaliables

typedef enum {

	UPDATE_TIMEOUT				// Update timeout timer

}timer_name_enum;


typedef struct {

	_Bool		lock;			/**< Flag que indica si el timer esta bloqueado (se esta usando actualmente) */
	uint64_t	target_time;		/**< Tiempo global calculado para que se cumpla el tiempo del timer */

}timer_struct;

_Bool 	check_timer(uint8_t id_timer);
void 	init_timers(void);
_Bool 	set_timer(uint8_t id_timer, uint32_t time_ms);
void 	stop_timer(void);
void 	unlock_timer(uint8_t id_timer);

#endif /* INC_TIMERS_H_ */
