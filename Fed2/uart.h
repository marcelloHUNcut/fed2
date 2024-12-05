#ifndef UART_H_
#define UART_H_
/******************************************************************************
* Include files
******************************************************************************/
#include <inttypes.h>

/******************************************************************************
* Types
******************************************************************************/


/******************************************************************************
* Constants
******************************************************************************/


/******************************************************************************
* Macros
******************************************************************************/
//UART
#define BAUD9600 51

/******************************************************************************
* Global Function Declarations
******************************************************************************/
void uart0_init(uint16_t baud);
void uart_write(uint8_t data);




#endif /* UART_H_ */