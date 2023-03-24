/*
 * update_controller.c
 *
 *  Created on: 9 feb. 2021
 *      Author: Bernar
 */
#include <boot_ctrl.h>
#include "flash_memory.h"
#include "timers.h"

update_struct update;

/*
 * Check if update is finished
 */
static _Bool is_update_finished(void)
{
	return (update.total_bytes > update.bytes_programmed)? 0 : 1;
}

/*
 * Check if there are update data received pending to save in flash memory
 */
static _Bool update_data_received(void)
{
	return (update.bytes_received - update.bytes_programmed) >= 4 ? 1 : 0;
}

/*
 * Check if checksum is correct
 */
static _Bool checksum_is_correct(void)
{
	return (update.checksum_calculated == update.checksum_received) ? 1 : 0;
}

/*
 * Erase update total bytes size
 */
static _Bool erase_old_app(void)
{
	_Bool res = 0;
	struct flash_memory *flash_info = NULL;

	flash_info->num_pages	= (update.total_bytes / FLASH_PAGE_SIZE) + 1;
	flash_info->page 		= APP_ADDRESS;
	flash_info->type_erase 	= TYPEERASE_PAGES;

	if(flash_erase(flash_info))
		res = 1;

	return res;
}

/*
 * Send confirmation to PC
 */
static _Bool confirm_update(uint8_t update_state)
{
	_Bool res = 0;
	uint32_t i = 0;
	const char *ok = "OK";
	struct can_pkt can_info = {0};

	switch(update_state){
		case UPDATE_NO_INITIALIZED:
			// Send confirmation to PC. The app update is valid and the bootloader
			// is ready. Also set a 3 seconds timeout if nothing is received from PC.
			can_info.p_can 	= 1;
			can_info.length = 0;
			can_info.id 	= update.id_can_pc;

			if(send_can_frame(&can_info)){
				unlock_timer(UPDATE_TIMEOUT);
				set_timer(UPDATE_TIMEOUT, 3000);
				res = 1;
			}
			break;
		case UPDATE_CHECKING:
			// Send confirmation to PC. The checksum is correct.
			can_info.p_can 	= 1;
			can_info.length = 0;
			can_info.id 	= update.id_can_pc;

			for(i = 0; i < 2; i++)
				can_info.buffer[i] = (uint8_t)ok[i];

			// Send OK to PC
			if(send_can_frame(&can_info)){
				unlock_timer(UPDATE_TIMEOUT);
				// Wait 100ms before reboot uC
				set_timer(UPDATE_TIMEOUT, 100);
				res = 1;
			}
			break;
		default:
			break;
	}

	return res;
}

/*
 * Reset update values
 */
static void boot_reset(void)
{
	uint32_t i;
	struct can_pkt can_info = {0};
	const char * timeout = "TIMEOUT";

	update.bytes_programmed 	= 0;
	update.bytes_received 		= 0;
	update.total_bytes 			= 0;
	update.checksum_calculated 	= 0;
	update.checksum_received 	= 0;
	update.index_programmed 	= 0;
	update.index_received 		= 0;
	update.state 				= UPDATE_NO_INITIALIZED;

	can_info.p_can 	= 1;
	can_info.id		= update.id_can_pc;
	can_info.length	= 7;

	for(i = 0; i < can_info.length; i++)
		can_info.buffer[i] = (uint8_t)timeout[i];

	send_can_frame(&can_info);
}

/*
 * Enable app mode setting BOOT_FLAG = 0.
 * uC will start in app mode the next time.
 */
static _Bool enable_app_mode(void)
{
	_Bool res = 0;

	struct flash_memory *flash_info = NULL;
	uint32_t flag_value = 0;

	flash_info->addr			= BOOT_FLAG;
	flash_info->type_program	= FLASH_TYPEPROGRAM_WORD;
	flash_info->data			= &flag_value;

	if(flash_write(flash_info))
		res = 1;

	return res;
}

/*
 * Enable boot mode setting BOOT_FLAG = 1.
 * uC will start in boot mode the next time.
 */
_Bool enable_boot_mode(void)
{
	_Bool res = 0;

	struct flash_memory *flash_info = NULL;

	flash_info->num_pages	= 1;
	flash_info->page 		= BOOT_FLAG;
	flash_info->type_erase 	= TYPEERASE_PAGES;

	if(flash_erase(flash_info))
		res = 1;

	return res;
}

 /*
  * Initialize peripherals
  */
void boot_init(void)
{
	init_timers();
	can_init();
	boot_reset();
}

/*
 * Bootloader machine state
 */
void boot_core(void)
{
	uint8_t i;
	uint32_t 	little_endian_word = 0;
	struct flash_memory *flash_info = NULL;

	switch(update.state){

		case(UPDATE_NO_INITIALIZED):
				// Do nothing
			break;
		case(UPDATE_INITIALIZED):
			// Check update timeout
			if(check_timer(UPDATE_TIMEOUT)){
				boot_reset();
				return;
			}
			if(update_data_received()){

				for(i = 0; i < 4; i++)
					little_endian_word |= update.buffer[update.index_programmed + i] << i*8;

				flash_info->data 			= &little_endian_word;
				flash_info->length 			= 1;
				flash_info->type_program 	= FLASH_TYPEPROGRAM_WORD;
				flash_info->addr 			= APP_ADDRESS + update.bytes_programmed;

				// Save in flash memory new data from update
				if(flash_write(flash_info)){
					// Update the index in the ring buffer and the total bytes programmed
			    	update.bytes_programmed += 4;
			    	update.index_programmed += 4;

			    	// Check the end of ring buffer
			    	if(update.index_programmed >= UPDATE_BUFFER_SIZE)
			    		update.index_programmed = 0;

			    	// Check if all the data for the update has been received
			    	if((is_update_finished()))
			    		update.state = UPDATE_WAITING_CHECKSUM;
				}
			}
			break;
		case(UPDATE_WAITING_CHECKSUM):
			if(check_timer(UPDATE_TIMEOUT))
				boot_reset();
			break;
		case(UPDATE_CHECKING):
			// Check if checksum received is equal to checksum calculated. If it is,
			// confirm update is correct to PC.
			if(checksum_is_correct() && confirm_update(UPDATE_CHECKING))
				update.state = UPDATE_REBOOT;
			else
				boot_reset();

			break;
		case(UPDATE_REBOOT):
			// Wait 100ms until reboot
			if(check_timer(UPDATE_TIMEOUT)){
				// Flag = 0 --> Jump to App
				enable_app_mode();
				// Reset uC
				HAL_NVIC_SystemReset();
			}
			break;
		default:
			break;
	}
}

/*
 * Jump from bootloader to app
 */
void jump_boot_to_app(void){
	/*
	 * Initialize the entry point.
	 * The entry point is the memory address where starts the principal app.
	 */
	void_type_function jump = (void_type_function)(*(__IO uint32_t*)(APP_ADDRESS + 4));

	// Deinitialize clock and peripherals
	HAL_RCC_DeInit();
	HAL_DeInit();

	// Reset clock variables
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL  = 0;

	// Set vector table register to principal app memory address
	SCB->VTOR = APP_ADDRESS;

	// Set stack pointer to new vector table
	__set_MSP(*(__IO uint32_t*)APP_ADDRESS);

	// Jump to app
	jump();

}

/*
 * Read the boot frame received from PC through the CAN bus
 */
void read_update_msg(struct can_pkt *can_rx_info){

	struct can_pkt can_tx_info = {};
	uint8_t type_frame	= can_rx_info->id & 0xF;
	uint8_t i;
	const char * alive = "ALIVE";

	switch(type_frame) {

		case REQUEST_UPDATE:
			// The frame type REQUEST_UPDATE contains the total size of the update.
			for(i = 0; i < can_rx_info->length; i++)
				update.total_bytes += can_rx_info->buffer[i] << i*8;

			update.id_can_pc = can_rx_info->id;

			// Check update size. Then enable boot mode, erase old app and
			// confirm the update.
			if(update.total_bytes < APP_MAX_SIZE && enable_boot_mode()
					&& erase_old_app() && confirm_update(UPDATE_NO_INITIALIZED))
					update.state 		= UPDATE_INITIALIZED;
			break;
		case UPDATE_DATA:
			// After receive new data, reset the timeout timer
			unlock_timer(UPDATE_TIMEOUT);
			set_timer(UPDATE_TIMEOUT, 3000);

			// Save the new data in ring buffer
			memcpy(&update.buffer[update.index_received], can_rx_info->buffer, can_rx_info->length);

			// Update the ring buffer index and the count of bytes received
			update.bytes_received += can_rx_info->length;
			update.index_received += can_rx_info->length;

			// Update de checksum calculated until now
			for(i = 0; i < can_rx_info->length; i++)
				update.checksum_calculated ^=  can_rx_info->buffer[i];

			// Check for the end of ring buffer
			if(update.index_received >= UPDATE_BUFFER_SIZE)
				update.index_received = 0;
			break;
		case CONFIRM_CHECKSUM:
			// After receive new data, reset the timeout timer
			unlock_timer(UPDATE_TIMEOUT);
			set_timer(UPDATE_TIMEOUT, 3000);

			// Save the value of checksum received
			update.checksum_received = *(uint32_t *)can_rx_info->buffer;
			// In the next state, check if checksum received is equal to checksum calculated
			update.state = UPDATE_CHECKING;
			break;
		case ALIVE:
			// Configure the Alive can packet
			can_tx_info.length	= 5;
			can_tx_info.id		= can_rx_info->id;
			can_tx_info.p_can	= 1;

			for(i = 0; i < can_tx_info.length; i++)
				can_tx_info.buffer[i] = (uint8_t)alive[i];

			// Send "ALIVE" to PC
 			send_can_frame(&can_tx_info);
			break;
		default:
			break;
	}
}

/*
 * Check if flag it's in app mode
 */
_Bool is_app_mode(void)
{
	return (flash_read_uint32(BOOT_FLAG)) ? 0 : 1;
}



