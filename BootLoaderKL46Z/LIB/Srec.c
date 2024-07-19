#include<stdio.h>
#include<stdint.h>
#include<math.h>
#include<stdlib.h>
#include"Srec.h"

SrecData_t SrecData ={0} ;
uint8_t Data_Rx[255],Data_RxInter[255];
uint8_t LEN, LENInter,h;
uint8_t ChartoHex(char c)
{
	uint8_t result;
	if ('0' <= c && c<= '9' )
	{
		result = c -'0';
	}
	else if ( 'A' <= c && c <= 'F')
	{
		result = c - 'A' + 10 ;
	}
	else
		result = 255;
	return result;
}


void StrtoHex_32(uint8_t* Arr, uint8_t len, uint32_t* result)
{
	uint8_t idx;
	*result = 0;
	for(idx = 0; idx < len; idx ++)
		*result = *result + ChartoHex(Arr[idx]) * pow(16,len - 1 - idx);
}
void StrtoHex_8(uint8_t* Arr, uint8_t len, uint8_t* result)
{
	uint8_t idx;
	*result = 0;
	for(idx = 0; idx < len; idx ++)
		*result = *result + ChartoHex(Arr[idx]) * pow(16,len - 1 - idx);
}

SrecStatus_t CheckLine()
{
//	SrecData.ByteCount = 0;
	SrecStatus_t status  = SREC_TRUE;
	uint8_t i;
	// check file Srec
	if (Data_Rx[0] != 'S' )
		status = SREC_ERR_FORMAT;
	for (i = 1; i < LEN - 2; i++)
		if ( ChartoHex(Data_Rx[i]) == 255)
		{
			status = SREC_ERR_FORMAT;
			break;
		}
	if ( status == SREC_TRUE)
	{
		// check Address
		uint8_t AddressNum = 0;
		if ('0' <= Data_Rx[1] && Data_Rx[1] <= '9' && Data_Rx[1] != 4)
		{
			if (('0' == Data_Rx[1]) || (Data_Rx[1] == '1'))
			{
				AddressNum = 2;// 2 bytes
			}
			else if(('2' == Data_Rx[1]) || (Data_Rx[3] == '1'))
			{
				AddressNum = ChartoHex(Data_Rx[1]) + 1 ;
			}
			else if(('7' == Data_Rx[1]) || (Data_Rx[1] == '8')|| (Data_Rx[1] == '9'))
			{
				AddressNum = 11 -  ChartoHex(Data_Rx[1]) ;
			}
			StrtoHex_32(&Data_Rx[4], AddressNum*2, &SrecData.Address);
		}
		else status = SREC_ERR_FORMAT;
		if (status == SREC_TRUE)
		{
			// Check ByteCounts
			StrtoHex_32(&Data_Rx[2],2 , &SrecData.ByteCount);
			if ((SrecData.ByteCount *2)  != (LEN - 2 - 4))
				status = SREC_ERR_BYTECOUNT;
			// Data && checksum
			if (status == SREC_TRUE)
			{
				// Data
				uint8_t idx;
				uint8_t i = 0;
				uint8_t Inter;
				if (('0' <= Data_Rx[1]) && (Data_Rx[1] <= '3'))
				{
					SrecData.DataNum = (SrecData.ByteCount - AddressNum -1)/4;// 1 byte of checksum
					for (idx = (2 + AddressNum) * 2; idx < LEN - 4 ; idx = idx + 8 )
					{
						StrtoHex_32(&Data_Rx[idx],8 , &SrecData.pData[i++]);
					}
				}
				else
				{
					 SrecData.DataNum = 0;
					 SrecData.AddStart = SrecData.Address;
				}
				// checkSum
				uint8_t ResultSum = 0 ;
				StrtoHex_8(&Data_Rx[LEN - 4], 2, &SrecData.CheckSum);
				ResultSum +=  SrecData.ByteCount;
				for (i=0; i< AddressNum; i++)
				{
					ResultSum +=(uint8_t) (SrecData.Address >> (i*8));
				}
				for (idx = (2 + AddressNum) * 2; idx < LEN - 4 ; idx = idx + 2 )
				{
					StrtoHex_8(&Data_Rx[idx],2 , &Inter);
					ResultSum +=Inter;
				}
				ResultSum += SrecData.CheckSum;
				if (255 != ResultSum )
				{
					status = SREC_ERR_CHECKSUM;
				}
			}

		}
		if (status == SREC_TRUE )
		{
			if(Data_Rx[1] == '0')
			{
				status = SREC_FIST_LINE;
			}
			else if ( ('7' == Data_Rx[1]) || (Data_Rx[1] == '8') || (Data_Rx[1] == '9'))
			{
				status = SREC_LAST_LINE;
			}
			else;
		}
	}
	return status;
}

