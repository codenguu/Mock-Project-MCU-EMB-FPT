/*
 * S_REC_LOAD_FLASH.h
 *
 *  Created on: Jul 14, 2024
 *      Author: SF
 */

#ifndef S_REC_LOAD_FLASH_H_
#define S_REC_LOAD_FLASH_H_

#include "stdint.h"

typedef enum{
	S0 = '0',
	S1 = '1',
	S2 = '2',
	S3 = '3',
	S4 = '4',
	S5 = '5',
	S6 = '6',
	S7 = '7',
	S8 = '8',
	S9 = '9'
}S_type;

typedef struct
{
	S_type	Record_type;
	uint32_t Address;
	uint8_t Byte_count;
	uint32_t Data;
	uint8_t CheckSum;
}Record_structure_t;

typedef enum{
	SREC_STATUS_LOAD_SUCCESS = 0U,
	SREC_STATUS_LOAD = 1U,
	SREC_STATUS_LOAD_ERR
}srec_status_t;
typedef enum{
	SREC_STATE_START	= 0U,
	SREC_STATE_TYPE,
	SREC_STATE_BYTE_COUNT,
	SREC_STATE_ADDRESS,
	SREC_STATE_DATA,
	SREC_STATE_CHECKSUM,
	SREC_STATE_ERR,
	SREC_STATE_DONE
}srec_state_t;

srec_status_t srec_Process_Load(uint8_t Data);

#endif /* S_REC_LOAD_FLASH_H_ */
