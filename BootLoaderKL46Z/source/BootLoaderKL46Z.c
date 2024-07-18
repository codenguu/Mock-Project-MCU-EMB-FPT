/**
 * @file    BootLoaderKL46Z.c
 * @brief   Application entry point.
 */

/* TODO: insert other include files here. */
#include "MKL46Z4.h"
#include "S_REC_LOAD_FLASH.h"
#include "flash.h"
#include "UART.h"
#include "User_port.h"
#include "User_gpio.h"
#include "SysTick.h"
#include "Circularqueue.h"
#include "BootLoaderKL46Z.h"

/*DEFINE OTHER VARIABLE */
#define QUEUE_ARRAY_SREC_HANDLE	(100U)

typedef enum
{
	false = 0U,
	true = 1U
}bool;

/*___________________Variable____________________*/
/*BUTTON*/
PORT_Config_t Button_PortConfig;
GPIO_Config_Type Button_GpioConfig;

/*PORT UART*/
PORT_Config_t UART_Tx_PortConfig;
PORT_Config_t UART_Rx_PortConfig;

/*___________________PROTOTYPE___________________*/
static void Button_Init(void);
static void UART_Config(uint32_t baudrate);
void UART_Handle_ISR(void);
bool Check_APP_Valid(uint32_t Address);
void Bootloader(uint32_t App_Address);
void Deinit_Peripherals(void);
void data_copy(unsigned int romstart, unsigned int start, unsigned int len);

/*___________________DATA_ASREC_HANDLE___________________*/
uint8_t QueueArrray[QUEUE_ARRAY_SREC_HANDLE] = {0};
CircurlarQueue_type CirQueue = {
		.Front = -1,
		.Rear = -1,
		.Size = QUEUE_ARRAY_SREC_HANDLE,
		.QueueArr = &QueueArrray[0]
};

extern uint32_t sizecode;

/*___________________State MODE___________________*/
typedef enum
{
	APPLICATION_MODE = 0U,
	BOOTLOADER_MODE = 1U,
	DEFAULT_MODE
}Init_mode_t;

static Init_mode_t init_mode = APPLICATION_MODE;
static srec_status_t status = SREC_STATUS_LOAD;

/*_____________________MAIN_______________________*/

int main(void) {
	//Initialization UART
	UART_Config(115200);

	//Initialization SW3
	Button_Init();

	Delay(100);
	//Check button to mode Boot
	if(GPIO_ReadPin(GPIOC, 3) == 0)
	{
		init_mode = BOOTLOADER_MODE;
		__disable_irq();
		if(Check_APP_Valid(APPLICATION_ADDRESS))
		{
			//Erase flash before copy
			Flash_Erase_nSector(FIRMWARE_BACKUP, FIRMWARE_BACKUP_END - 0x400);
			//Copy old firmware to backup region
			data_copy(APPLICATION_ADDRESS, FIRMWARE_BACKUP, *(uint32_t*)(FLASH_ADDRESS_END - 0x400));
		}
		//Erase flash before into Boot mode
		Flash_Erase_nSector(APPLICATION_ADDRESS, FLASH_ADDRESS_END - 1);
		UART_SendString("Send SREC file:", 15);
		__enable_irq();
	}

	while(1)
	{
		switch(init_mode)
		{
			case DEFAULT_MODE:

				break;
			case BOOTLOADER_MODE:
				if(CircurlarQueue_IsEmpty(&CirQueue))
				{
					break;
				}
				uint8_t Datapop = CircurlarQueue_PopData(&CirQueue);
				status = srec_Process_Load(Datapop);
				if(status == SREC_STATUS_LOAD_SUCCESS)
				{
					init_mode = APPLICATION_MODE;
				}
				else if(status == SREC_STATUS_LOAD_ERR)
				{
					init_mode = DEFAULT_MODE;
				}
				break;
			case APPLICATION_MODE:
				//Check see that have not empty
				if(Check_APP_Valid(APPLICATION_ADDRESS))
				{
					//Jump to Application
					UART_SendString("Firmware Run!", 13);
					Bootloader(APPLICATION_ADDRESS);
				}
				else if(Check_APP_Valid(FIRMWARE_BACKUP))	//Check Firmware backup invalid
				{
					//Jump to Application
					UART_SendString("Firmware backup Run!", 20);
					Bootloader(FIRMWARE_BACKUP);
				}
				else
				{
					UART_SendString("Empty! Let's Update firmware!", 30);
					UART_SendString("keep SW3 + reset to load firmware", 33);
					init_mode = DEFAULT_MODE;
				}
				break;
		}
	}
    return 0 ;
}

/*Check Application is valid */
bool Check_APP_Valid(uint32_t Address)
{
	if(*(volatile uint32_t *)Address == 0xFFFFFFFF)
	{
		return false;
	}
	return true;
}

/*Boot loader*/
void Bootloader(uint32_t App_Address)
{
	//Disable Interrupt
	__disable_irq();
	Deinit_Peripherals();

	//Set main stack pointer
	__set_MSP(*(volatile uint32_t*)App_Address);

	//ensuring this instruction is completed
	__DMB();
	SCB->VTOR = App_Address;
	__DSB();

	//Set PC to ResetHandler of firmware
	uint32_t Address_reset_handler = *(volatile uint32_t *)(App_Address + 4);
	void (*reset_handler)(void) = (void(*)(void))Address_reset_handler;

	//Jump to Application
	reset_handler();
}

/*_____________________UART HANDLE ISR_____________________*/
void UART_Handle_ISR(void)
{
	//New Method
	CircurlarQueue_PushData(&CirQueue, UART0->D);
}

/*_______________________COP FLASH_______________________*/

// Copy data from source address to destination address
void data_copy(unsigned int romstart, unsigned int start, unsigned int len) {
	unsigned int *pulDest = (unsigned int*) start;
	unsigned int *pulSrc = (unsigned int*) romstart;
	unsigned int loop;
	for (loop = 0; loop < len; loop = loop + 4)
	{
		Flash_WriteLittle((uint32_t)pulDest++, *pulSrc++);
	}
}

/*_____________________HARDWARE INIT_____________________ */
static void Button_Init(void)
{
	//Port
	Button_PortConfig.PORTx = PORTC;
	Button_PortConfig.PIN = 3;
	Button_PortConfig.irqc = PORT_INTERRUPT_DISABLE;
	Button_PortConfig.mux = PORT_MUX_GPIO;
	Button_PortConfig.pull = PORT_PULL_UP;
	//Gpio
	Button_GpioConfig.GPIOx = GPIOC;
	Button_GpioConfig.PIN = 3;
	Button_GpioConfig.pddr = GPIO_INPUT;
	CLOCK_PortInit(PORTC);
	PORT_Pin_Init(&Button_PortConfig);
	GPIO_Init(&Button_GpioConfig);
	PORT_EXTI_Config(&Button_PortConfig);
}

static void UART_Config(uint32_t baudrate)
{
	//
	UART_Rx_PortConfig.PORTx = PORTA;
	UART_Rx_PortConfig.PIN = 1;
	UART_Rx_PortConfig.mux = PORT_MUX_ALTERNATIVE2;	//UART_RX.
	UART_Rx_PortConfig.pull = PORT_PULL_UP;
	//
	UART_Tx_PortConfig.PORTx = PORTA;
	UART_Tx_PortConfig.PIN = 2;
	UART_Tx_PortConfig.mux = PORT_MUX_ALTERNATIVE2;	//UART_TX.
	UART_Tx_PortConfig.pull = PORT_PULL_UP;

	UART_Init(&UART_Tx_PortConfig, &UART_Rx_PortConfig, UART_Handle_ISR, baudrate);
}
/*_____________________DEINIT_____________________ */
void Deinit_Peripherals(void)
{
	// Disable clock to UART0 module
	SIM->SOPT2 &= ~SIM_SOPT2_UART0SRC_MASK;
	SIM->SCGC4 &= ~SIM_SCGC4_UART0_MASK;
    SIM->SCGC5 &= ~SIM_SCGC5_PORTA_MASK;

	// Disable clock to PORTA,C module
	SIM->SCGC5 &= ~SIM_SCGC5_PORTC_MASK;
}
