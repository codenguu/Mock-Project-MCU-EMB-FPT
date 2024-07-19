/*
 * S_REC_LOAD_FLASH.c
 *
 *  Created on: Jul 14, 2024
 *      Author: SF
 */
#include "S_REC_LOAD_FLASH.h"
#include "UART.h"
#include "flash.h"
#include "BootLoaderKL46Z.h"
#define HEX_INVALID	 	(0xFFU)
#define false			(0U)
#define true			(1U)

//size code
static uint32_t sizecode = 0;

uint8_t hex(uint8_t hex);		//Char to hex

//__________Variable__________
static srec_state_t state = SREC_STATE_START;
static Record_structure_t Data_Rec = {0};
static uint32_t Data_tmpfirstAddr;

volatile int16_t 	line = 0;
volatile uint8_t 	number_Address_hex_digit = 0;
volatile uint8_t 	number_Data_hex_digit;
volatile uint8_t 	number_Data_4Byte = 0;

//Flag
volatile uint8_t S_ok			= 0;
volatile uint8_t Type_ok		= 0;
volatile uint8_t ByteCount_ok 	= 0;
volatile uint8_t Address_ok		= 0;
volatile uint8_t Data_ok		= 0;
volatile uint8_t CheckSum_ok	= 0;
volatile uint8_t Load_confirm	= 0;

srec_status_t srec_Process_Load(uint8_t Data)
{
	srec_status_t status = SREC_STATUS_LOAD;
	switch(state)
	{
		case SREC_STATE_START:
			line++;
			if(Data != 'S')
			{
				state = SREC_STATE_ERR;
				break;
			}
			state = SREC_STATE_TYPE;
			break;
		case SREC_STATE_TYPE:
			Data_Rec.Record_type = Data;
			switch(Data_Rec.Record_type)
			{
				case S0:
				case S5:
				case S9:
					number_Address_hex_digit = 4;//hex
					Load_confirm = 0;
					break;
				case S1:
					number_Address_hex_digit = 4;//hex
					Load_confirm = 1;
					break;
				case S2:
					number_Address_hex_digit = 6;//hex
					Load_confirm = 1;
					break;
				case S3:
					number_Address_hex_digit = 8;//hex
					Load_confirm = 1;
					break;
				case S7:
					number_Address_hex_digit = 8;//hex
					Load_confirm = 0;
					break;
				case S8:
					number_Address_hex_digit = 6;//hex
					Load_confirm = 0;
					break;
				default:
					state = SREC_STATE_ERR;
					break;
			}
			state = SREC_STATE_BYTE_COUNT;
			break;
		case SREC_STATE_BYTE_COUNT:
			Data_Rec.Byte_count = (Data_Rec.Byte_count << 4) | hex(Data);
			ByteCount_ok++;
			if(ByteCount_ok == 2)
			{
				Data_Rec.CheckSum = Data_Rec.Byte_count;
				state = SREC_STATE_ADDRESS;
			}
			break;
		case SREC_STATE_ADDRESS:
			Data_Rec.Address = (Data_Rec.Address << 4) | hex(Data);
			Address_ok++;
			if(Address_ok == number_Address_hex_digit)
			{
				Data_Rec.CheckSum = (uint8_t)(Data_Rec.CheckSum + (Data_Rec.Address >> 24) +
						(Data_Rec.Address >> 16) + (Data_Rec.Address >> 8) + (uint8_t)Data_Rec.Address);
				state = SREC_STATE_DATA;
				if((Data_Rec.Record_type == S9) ||( Data_Rec.Record_type == S8) ||(Data_Rec.Record_type == S7))
				{
					Data_Rec.CheckSum = (0xFF - Data_Rec.CheckSum );
					state = SREC_STATE_CHECKSUM;
				}
			}
			break;
		case SREC_STATE_DATA:
			number_Data_hex_digit = Data_Rec.Byte_count*2 - number_Address_hex_digit - 2;	//Byte count = address + data + checksum
			Data_Rec.Data = (Data_Rec.Data << 4) | hex(Data);
			Data_ok++;
			if(!(Data_ok % 2))
			{
				Data_Rec.CheckSum +=(uint8_t)(Data_Rec.Data);
			}
			if(!(Data_ok%8))
			{
				if(Data_Rec.Address == APPLICATION_ADDRESS && Data_ok == 8)	//Skip load to flash 4 byte first address
				{
					Data_tmpfirstAddr = Data_Rec.Data;
					number_Data_4Byte++;
				}
				else if(Load_confirm)
				{
					Flash_Write(Data_Rec.Address +number_Data_4Byte*4, Data_Rec.Data);
					number_Data_4Byte++;
					sizecode+=4;
				}
			}
			if(Data_ok == number_Data_hex_digit)
			{
				Data_Rec.CheckSum = (0xFF - Data_Rec.CheckSum);
				state = SREC_STATE_CHECKSUM;
			}
			break;
		case SREC_STATE_CHECKSUM:
			CheckSum_ok++;
			if(CheckSum_ok == 1)
			{
				if((Data_Rec.CheckSum >> 4) != hex(Data))
				{
					state = SREC_STATE_ERR;
				}
			}
			else if(CheckSum_ok == 2)
			{
				if((Data_Rec.CheckSum & 0xF) == hex(Data))
				{
					state = SREC_STATE_DONE;
				}
			}
			break;
		case SREC_STATE_DONE:
			//Clear Flag
			if(Data == '\r')
			{
				S_ok = 0;
				Type_ok	= 0;
				ByteCount_ok = 0;
				Address_ok	= 0;
				Data_ok	= 0;
				CheckSum_ok	= 0;
				Load_confirm = 0;
				number_Data_4Byte = 0;
				//Clear buffer
				Data_Rec.Address = 0;
				Data_Rec.Byte_count = 0;
				Data_Rec.CheckSum = 0;
				Data_Rec.Data = 0;
			}
			if(Data == '\n')		// 'r'
			{
				state = SREC_STATE_START;
			}
			if((Data_Rec.Record_type == S9) ||( Data_Rec.Record_type == S8) ||(Data_Rec.Record_type == S7))
			{
				//Write 4 byte head
				Flash_Write(APPLICATION_ADDRESS, Data_tmpfirstAddr);

				//Save size to back up
				sizecode+=4;
				Flash_Erase(FLASH_ADDRESS_END -4);
				Flash_WriteLittle(FLASH_ADDRESS_END - 4, sizecode);
				UART_SendString("Load firmware successfully !",28);
				UART_RecverInterruptDisable();
				status = SREC_STATUS_LOAD_SUCCESS;
			}
			break;
		case SREC_STATE_ERR:
			UART_RecverInterruptDisable();
			UART_SendString("Error Load firmware in line ",29);
			UART_SendChar((uint8_t)(line/1000) + 0x30);
			UART_SendChar((uint8_t)(line/100%10) + 0x30);
			UART_SendChar((uint8_t)(line/10%10)+ 0x30);
			UART_SendChar((uint8_t)(line%10) + 0x30);

			UART_SendString("Press reset to jump to backup firmware",38);
			status = SREC_STATUS_LOAD_ERR;
			break;
	}
	return status;
}


//'S','2','1','4','0','1','0','0','0','0','0','0','6','0','0','0','2','0'
//,'D','5','0','0','0','1','0','0','4','3','0','1','0','1','0','0','4','5',
//'0','1','0','1','0','0','0','8','\r','\n'


//Convert a character to hex
uint8_t hex(uint8_t hex)
{
	if (hex >= '0' && hex <= '9')
		return hex - '0';
	else if (hex >= 'A' && hex <= 'F')
		return hex - 'A' + 10;
	else if (hex >= 'a' && hex <= 'f')
		return hex - 'a' + 10;
	else
		return HEX_INVALID;
}



















