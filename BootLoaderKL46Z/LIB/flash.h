#ifndef FLASH_H_
#define FLASH_H_

#include "MKL46Z4.h"

void Flash_Write(uint32_t Flash_Address, uint32_t fl_data);
void Flash_Erase(uint32_t Flash_Address);
void Flash_Erase_nSector(uint32_t Start_Flash_Address, uint32_t End_Flash_Address);
void Flash_WriteLittle(uint32_t Flash_Address, uint32_t fl_data);
#endif /* FLASH_H_ */
