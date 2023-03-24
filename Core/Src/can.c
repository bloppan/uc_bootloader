/**
  ******************************************************************************
  * @file    can.c
  * @author  Bernardo López Panadero
  * @brief   Gestion de funciones de la comunicación CAN
  *
  ******************************************************************************
  */

#include <boot_ctrl.h>
#include "can.h"
#include "flash_memory.h"
#include <stdio.h>
#include "stm32f1xx_hal.h"
#include <string.h>

extern update_struct update;

extern CAN_HandleTypeDef hcan;
CAN_RxHeaderTypeDef		rx_message;
CAN_TxHeaderTypeDef 	tx_message;

// Two arrays for rx and tx can packets
struct can_pkt can_rx[CAN_BUFFER_SIZE] = {0};
struct can_pkt can_tx[CAN_BUFFER_SIZE] = {0};

// Index for rings buffers
static uint16_t index_rx_pending	= 0;
static uint16_t index_rx_processed	= 0;
static uint16_t	index_tx_sent		= 0;
static uint16_t	index_tx_pending	= 0;

static void		process_can_pkt(struct can_pkt *can_rx_pkt);
static void 	process_ctrl_msg(struct can_pkt *can_rx_pkt);
static _Bool	try_send_can_frame(struct can_pkt *can_tx_frame);

/**
* Always checking if there are any can frames received and pending to process
*/
void can_core_rx(void)
{
	if(can_rx[index_rx_processed].is_pending){
		// Process can packet if it is pending
		process_can_pkt(&can_rx[index_rx_processed]);
		can_rx[index_rx_processed].is_pending = 0;
		// Increment the index of the CAN packet ring buffer.
		index_rx_processed++;

		if(index_rx_processed >= CAN_BUFFER_SIZE)
			index_rx_processed = 0;
	}
}
/**
 * Always checking if there are any can frames pending to send
 */
void can_core_tx(void)
{
	// Check if there are any TX packets pending and try to send them.
	if(can_tx[index_tx_sent].is_pending && try_send_can_frame(&can_tx[index_tx_sent])){

		can_tx[index_tx_sent].is_pending = 0;
		// Update the index of the TX CAN packet ring buffer.
		index_tx_sent++;

		if(index_tx_sent >= CAN_BUFFER_SIZE)
			index_tx_sent = 0;
	}
}

/**
 * Stop the can bus
 */
void stop_can_bus(uint8_t p_can)
{
	// Stop CAN
	if(HAL_CAN_Stop(&hcan) != HAL_OK)
		Error_Handler();
}

/**
* Initialize CAN bus
*/
void can_init(void)
{
	if(HAL_CAN_Start(&hcan) != HAL_OK)
		Error_Handler();

	if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
		Error_Handler();
}

/**
 * Process control message
*/
static void process_ctrl_msg(struct can_pkt *can_rx_pkt)
{
	uint8_t ctrl_cmd = can_rx_pkt->id;

	switch(ctrl_cmd) {
		// Request GO BOOT
		case TYPE_GO_BOOT:
			enable_boot_mode();		// Set BOOT_FLAG = 1
			HAL_NVIC_SystemReset(); // Reset uC
			break;
		default:
			break;
	}
}

/**
* Process can frame pending to read
*/
static void process_can_pkt(struct can_pkt *can_rx_pkt)
{
	// Get packet type from can ID
	uint8_t pkt_type = (can_rx_pkt->id >> 8);

	switch(pkt_type){
		case TYPE_CONTROL:
			process_ctrl_msg(can_rx_pkt);	// Go BOOT
			break;
		case TYPE_UPDATE:
			read_update_msg(can_rx_pkt);	// Data received from update
			break;
		default:
			break;
		}
}

/**
 * Try to send a can frame
 */
static _Bool try_send_can_frame(struct can_pkt *can_tx_info)
{
	_Bool		res = 0;
	uint32_t 	TxMailbox;

	if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan) > 0){

		tx_message.StdId				= can_tx_info->id;
		tx_message.IDE 					= CAN_ID_STD;
		tx_message.RTR 					= CAN_RTR_DATA;
		tx_message.DLC			 		= can_tx_info->length;
		tx_message.TransmitGlobalTime 	= DISABLE;
		// Try send
		if (HAL_CAN_AddTxMessage(&hcan, &tx_message, can_tx_info->buffer, &TxMailbox) == HAL_OK)
			res = 1;
	}

	return res;
}

/**
 * Call send_can_frame function to queue a can frame
 */
_Bool send_can_frame(struct can_pkt *can_tx_info)
{
	_Bool ret = 0;

	// Check if the can packet ring buffer is full
	if(!can_tx[index_tx_pending].is_pending){

		// Save the config of can tx packet
		can_tx[index_tx_pending].p_can		= can_tx_info->p_can;
		can_tx[index_tx_pending].id			= can_tx_info->id;
		can_tx[index_tx_pending].length		= can_tx_info->length;
		can_tx[index_tx_pending].is_pending = 1;

		// Save data of can tx packet
		memcpy(can_tx[index_tx_pending].buffer, can_tx_info->buffer, can_tx_info->length);

		index_tx_pending++;
		// Check if the can tx packet ring buffer is full
		if(index_tx_pending >= CAN_BUFFER_SIZE)
			index_tx_pending = 0;

		ret = 1;
	}

	return ret;
}

/**
 * RX Callback CAN msg received
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_message, can_rx[index_rx_pending].buffer) == HAL_OK){

		// Save the info on new CAN packet pending to read
		can_rx[index_rx_pending].length		= rx_message.DLC; //Comprobar si hay que hacer >> 16
		can_rx[index_rx_pending].id 		= rx_message.StdId;
		can_rx[index_rx_pending].is_pending = 1;
		can_rx[index_rx_pending].p_can		= 1;
		// Update the index of the RX CAN pakcet ring buffer
		index_rx_pending++;

		if(index_rx_pending >= CAN_BUFFER_SIZE)
			index_rx_pending = 0;
	}
}

/**
 * RX Callback CAN msg received
 */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &rx_message, can_rx[index_rx_pending].buffer) == HAL_OK){

		// Save the info on new CAN packet pending to read
		can_rx[index_rx_pending].length			= rx_message.DLC; //Comprobar si hay que hacer >> 16
		can_rx[index_rx_pending].id 			= rx_message.StdId;
		can_rx[index_rx_pending].is_pending 	= 1;
		can_rx[index_rx_pending].p_can			= 1;
		// Update the index of the RX CAN pakcet ring buffer
		index_rx_pending++;

		if(index_rx_pending >= CAN_BUFFER_SIZE)
			index_rx_pending = 0;
	}
}





