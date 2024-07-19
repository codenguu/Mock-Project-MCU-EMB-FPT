#include "flash.h"

void Flash_Write(uint32_t Flash_Address, uint32_t fl_data)
{
	//Wait previous command complete
	while((FTFA->FSTAT & FTFA_FSTAT_CCIF_MASK) == 0);

	//Check error from previous command
	if((FTFA->FSTAT & FTFA_FSTAT_ACCERR_MASK) || (FTFA->FSTAT & FTFA_FSTAT_FPVIOL_MASK))
	{
		FTFA->FSTAT = 0x30;
	}

	FTFA->FCCOB0 = 0x06;	//CMD Write flash
	//Address
	FTFA->FCCOB1 = (Flash_Address >> 16) & 0xFF;
	FTFA->FCCOB2 = (Flash_Address >> 8) & 0xFF;
	FTFA->FCCOB3 = Flash_Address & 0xFF;
	//Data
	FTFA->FCCOB4 = (uint8_t)fl_data;
	FTFA->FCCOB5 = (uint8_t)(fl_data >> 8);
	FTFA->FCCOB6 = (uint8_t)(fl_data >> 16);
	FTFA->FCCOB7 = (uint8_t)(fl_data >> 24);

	//Clear the CCIF to launch the command
	FTFA->FSTAT = 0x80;
	while((FTFA->FSTAT & FTFA_FSTAT_CCIF_MASK) == 0);
}

void Flash_WriteLittle(uint32_t Flash_Address, uint32_t fl_data)
{
	//Wait previous command complete
	while((FTFA->FSTAT & FTFA_FSTAT_CCIF_MASK) == 0);

	//Check error from previous command
	if((FTFA->FSTAT & FTFA_FSTAT_ACCERR_MASK) || (FTFA->FSTAT & FTFA_FSTAT_FPVIOL_MASK))
	{
		FTFA->FSTAT = 0x30;
	}

	FTFA->FCCOB0 = 0x06;	//CMD Write flash
	//Address
	FTFA->FCCOB1 = (Flash_Address >> 16) & 0xFF;
	FTFA->FCCOB2 = (Flash_Address >> 8) & 0xFF;
	FTFA->FCCOB3 = Flash_Address & 0xFF;
	//Data
	FTFA->FCCOB4 = (uint8_t)(fl_data >> 24);
	FTFA->FCCOB5 = (uint8_t)(fl_data >> 16);
	FTFA->FCCOB6 = (uint8_t)(fl_data >> 8);
	FTFA->FCCOB7 = (uint8_t)fl_data;

	//Clear the CCIF to launch the command
	FTFA->FSTAT = 0x80;
	while((FTFA->FSTAT & FTFA_FSTAT_CCIF_MASK) == 0);
}
void Flash_Erase(uint32_t Flash_Address)
{
	//Wait previous command complete
	while((FTFA->FSTAT & FTFA_FSTAT_CCIF_MASK) == 0);

	//Check error from previous command
	if((FTFA->FSTAT & FTFA_FSTAT_ACCERR_MASK) || (FTFA->FSTAT & FTFA_FSTAT_FPVIOL_MASK))
	{
		FTFA->FSTAT = 0x30;
	}

	FTFA->FCCOB0 = 0x09;
	FTFA->FCCOB1 = (Flash_Address >> 16) & 0xFF;
	FTFA->FCCOB2 = (Flash_Address >> 8) & 0xFF;
	FTFA->FCCOB3 = Flash_Address & 0xFF;

	//Clear the CCIF to launch the command
	FTFA->FSTAT = 0x80;
	//Wait previous command complete
	while((FTFA->FSTAT & FTFA_FSTAT_CCIF_MASK) == 0);
}

void Flash_Erase_nSector(uint32_t Start_Flash_Address, uint32_t End_Flash_Address)
{
	uint32_t numSectors = (End_Flash_Address - Start_Flash_Address + 1024 - 1) / 1024;
	for (uint32_t i = 0; i < numSectors; i++)
	{
		Flash_Erase(Start_Flash_Address + i * 1024);
	}
}
//void Flash_Write_nByte(uint32_t Flash_Address)
