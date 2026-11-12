#include "stm32c031xx.h"

#include "myuart.h"

/*

Character transmission procedure [STM32C0 REFERENCE MANUAL]

To transmit a character, follow the sequence below:

1. Program the M bits in USART_CR1 to define the word length. [DONE]
2. Select the desired baud rate using the USART_BRR register. [DONE]
3. Program the number of stop bits in USART_CR2. [DONE]
4. Enable the USART by writing the UE bit in USART_CR1 register to 1.[DONE]
5. Select DMA enable (DMAT) in USART_CR3 if multi-buffer communication must take
place. Configure the DMA register as explained in Section26.5.19: Continuous
communication using USART and DMA. [TBD]
6. Set the TE bit in USART_CR1 to send an idle frame as first transmission.[DONE]
7. Write the data to send in the USART_TDR register. Repeat this for each data to be
transmitted in case of single buffer.
– When FIFO mode is disabled, writing a data to the USART_TDR clears the TXE
flag.
– When FIFO mode is enabled, writing a data to the USART_TDR adds one data to the TXFIFO.
Write operations to the USART_TDR are performed when TXFNF flag is set. This flag remains set until the TXFIFO is full.
8. When the last data is written to the USART_TDR register, wait until TC=1.
– When FIFO mode is disabled, this indicates that the transmission of the last frame is complete.
– When FIFO mode is enabled, this indicates that both TXFIFO and shift register are empty.
This check is required to avoid corrupting the last transmission when the USART is
disabled or enters Halt mode.

*/

void initClock(void)

{
	/*
	 * peripherals are clock gated so need to enable clocks
	 * PA2 has alternate function USART2_TX
	 * PA3 has alternate function USART2_RX
	 * PORTA block is on AHB bus
	 * USART2 block is on APB bus
	*/

	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;									//	portA clock enable (bit 0)
	RCC->APBENR1 |= RCC_APBENR1_USART2EN; 								//	USART2 clock enable (bit 17)

	// read back the register to create a 1-cycle delay.
	// this guarantees the clock is stable before the CPU leaves this function.

    (void)RCC->APBENR1;
}

void configurePort(void)

{
	/*
	 * set PA2 and PA3 pin modes to alternate function 0b01
	*/

	GPIOA->MODER &= ~(GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk); 	//	clear mode bits for PA2 and PA3
	GPIOA->MODER |=(GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1); 			//	set mode bits for PA2 and PA3 as alternate function pins

	/*
	 * set alternate functions for PA2 and PA3 to USART2
	 * PA2 is UART2_TX (AF1: 0b0001)
	 * PA3 is UART2_RX (AF1: 0b0001)
	*/

	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_Msk | GPIO_AFRL_AFSEL3_Msk); 	//	clear alternate function select bits (AFSEL) for PA2 and PA3
	GPIOA->AFR[0] |= (GPIO_AFRL_AFSEL2_0 | GPIO_AFRL_AFSEL3_0); 		//	set AFSEL2 and AFSEL3 to AF1 for USART2_TX and USART2_RX
}

void configureUART(void)

{
	/*
	 *UART configure (configure these registers before setting USART_CR1_UE to 1)
	 *start bits: 1 [done]
	 *parity bits: 1
	 *data bits: 8 [done]
	 *parity bit: none
	 *baud rate: 115200
	*/

	USART2->CR1	&= ~(USART_CR1_M0_Msk 			//	1. clear M0 and M1 bits 28 and 12 (M[1:0]=0b00) for 1 start bit, 8 data bits, 'n' stop bits
				| USART_CR1_M1_Msk);
	USART2->BRR=104; 							//	2. set baud rate to 115200 (CLK=12MHz-> 12MHz/104~115200)
	USART2->CR2 &= ~(USART_CR2_STOP_Msk);		//	3. clear stop bits 13 and 12 (STOP[1:0]=0b00) for 1 stop bit

	/*
	 * after USART2 configuration is completed
	*/

	USART2->CR1 |= (USART_CR1_UE);				//	4. USART enable
	USART2->CR1 |= (USART_CR1_TE				//	5. transmitter enable
			| USART_CR1_RE);					//	6. receiver enable
}

void initUART(void)

{
	initClock();
	configurePort();
	configureUART();
}


void transmitUART(char dataTX)

{
	/*
	  * TXE flag (FIFO disabled) or TXFNF (FIFO enabled) is bit 7 of USART_ISR register
	  *
	  * TXE bit is cleared by HW when new data is written to USART_TDR/HW is busy
	  * TXE bit is set by HW when content of USART_TDR register has been transferred to TX shift register
	  *
	  * TXE/TXFNF = 0 - USART_TDR register is full i.e. HW is still busy
	  * TXE/TXFNF = 1 - USART_TDR register is empty i.e. content has been transferred to TX shift register
	 */

    while (!(USART2->ISR & USART_ISR_TXE_TXFNF));			//	wait here while transmit data register USART_TDR is full (blocks CPU)
    USART2->TDR = dataTX;									//	once while loop breaks, write exactly once

}

char receiveUART(void)

{
	/*
	  * RXNE flag (FIFO disabled) or RXFNE (FIFO enabled) is bit 5 of USART_ISR register
	  *
	  * RXNE bit is cleared by HW when data in USART_RDR has been read/no new data
	  * RXNE bit is set by HW when content of RX shift register has been transferred USART_RDR register
	  *
	  * RXNE/RXFNE = 0 - no data to read i.e. cleared by HW when USART_RDR register is empty/data has been read
	  * RXNE/RXFNE = 1 - data ready to read i.e. set by HW when USART_RDR register is full
	 */

    while (!(USART2->ISR & USART_ISR_RXNE_RXFNE));				//	wait here while receive data register USART_RDR is empty
    return (char)USART2->RDR;									//	once while loop breaks, write exactly once

}
