/*
 * update_controller.h
 *
 *  Created on: 9 feb. 2021
 *      Author: Bernar
 */

#ifndef INC_BOOT_CTRL_H_
#define INC_BOOT_CTRL_H_

#include "can.h"
#include "stm32f1xx_hal.h"
#include <stdlib.h>
#include <string.h>

#define UPDATE_BUFFER_SIZE		256

typedef enum {

	REQUEST_UPDATE,
	UPDATE_DATA,
	CONFIRM_CHECKSUM,
	ALIVE

}update_frame_enum;

typedef enum {

	UPDATE_NO_INITIALIZED,
	UPDATE_INITIALIZED,
	UPDATE_WAITING_CHECKSUM,
	UPDATE_CHECKING,
	UPDATE_REBOOT

}update_state_enum;

typedef struct {

	uint8_t		buffer[UPDATE_BUFFER_SIZE];
	uint32_t	bytes_programmed;
	uint32_t	bytes_received;
	uint32_t	total_bytes;
	uint32_t	checksum_calculated;
	uint32_t 	checksum_received;
	uint16_t	id_can_pc;
	uint8_t		index_programmed;
	uint8_t		index_received;
	uint8_t		state;

}update_struct;


// Pointer to void function
typedef void (*void_type_function)(void);

_Bool	is_app_mode(void);
_Bool 	enable_boot_mode(void);
void 	boot_init(void);
void 	boot_core(void);
void 	jump_boot_to_app(void);
void 	read_update_msg(struct can_pkt *can_rx_info);

#endif /* INC_BOOT_CTRL_H_ */


