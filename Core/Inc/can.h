/**
  ******************************************************************************
  * @file    can.h
  * @author  Bernardo López Panadero
  * @brief   Definiciones y funciones de la comunicación CAN
  *
  ******************************************************************************
  */
#ifndef __can_h
#define __can_h

#include "main.h"
#include "stm32f1xx_hal.h"

// Size of can packets buffer
#define	CAN_BUFFER_SIZE		100

// PERIPHERALS
#define	P_CAN_1				1

// FRAME TYPES
#define	TYPE_CONTROL		0x00
#define	TYPE_UPDATE			0x06
#define	TYPE_GO_BOOT		0x0A


struct can_pkt {

	uint8_t		buffer[8];		// Buffer size is 8 bytes
	uint16_t	id;				// Id packet
	_Bool		is_pending;		// is pending to send or process?
	uint8_t		length;			// Length
	uint8_t		p_can;			// Peripheral
};

void 	can_core_rx(void);
void	can_core_tx(void);
void 	can_init(void);
void 	stop_can_bus(uint8_t p_can);
_Bool 	send_can_frame(struct can_pkt *can_info);

#endif



