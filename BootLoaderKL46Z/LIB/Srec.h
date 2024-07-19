

#ifndef SREC_H_
#define SREC_H_

typedef enum{
	SREC_TRUE           = 0,
	SREC_ERR_FORMAT     = 1,
	SREC_ERR_MEMORY     = 2,
	SREC_ERR_CHECKSUM   = 3,
	SREC_ERR_BYTECOUNT  = 4,
	SREC_ERR            = 5,
	SREC_FIST_LINE      = 6,
	SREC_LAST_LINE      = 7
} SrecStatus_t;

typedef struct
{
	uint8_t SrecType;
	uint32_t ByteCount;
	uint32_t Address;
	uint32_t pData[10];
	uint8_t DataNum;
	uint8_t CheckSum;
	uint32_t AddStart;
} SrecData_t;

uint8_t ChartoHex(char c);
void StrtoHex_32(uint8_t*, uint8_t , uint32_t*);
void StrtoHex_8(uint8_t*, uint8_t, uint8_t* );
SrecStatus_t CheckLine();

#endif /* SREC_H_ */
