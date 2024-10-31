/******************************************************************************
 * Created: 
 * Author :
******************************************************************************/
 /******************************************************************************
* Include files
******************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#define F_CPU 8000000UL
#include <avr/delay.h>

#include <inttypes.h>
#include "lcd.h"
#include "uart.h"
#include "peripherals.h"

/******************************************************************************
* Macros
******************************************************************************/
#define TRUE 1
#define FALSE 0

/******************************************************************************
* Constants
******************************************************************************/


/******************************************************************************
* Global Variables
******************************************************************************/
uint8_t task_10ms =FALSE, task_100ms=FALSE, task_500ms=FALSE;
uint16_t timer_cnt=0;

double accel_pos = 0;
double decel_pos = 0;
double steer_angle = 0;
/******************************************************************************
* External Variables
******************************************************************************/
//teszteles
double pose [3] = {0,0,0};


/******************************************************************************
* Local Function Declarations
******************************************************************************/
void port_init(void);


/******************************************************************************
* Local Function Definitions
******************************************************************************/
/******************************************************************************
* Function:         void port_init(void)
* Description:      I/O portok inicializ?l?sa
* Input:
* Output:
* Notes:
******************************************************************************/
void port_init(void)
{	
	DDRC = (1<<LCD_E) | (1<<LCD_RS) | (1<<LCD_D7) | (1<<LCD_D6) | (1<<LCD_D5) | (1<<LCD_D4);
	PORTC = (0<<LCD_E) | (0<<LCD_RS) | (0<<LCD_D7) | (0<<LCD_D6) | (0<<LCD_D5) | (0<<LCD_D4);
}

/******************************************************************************
* Function:         int main(void)
* Description:      main function
* Input:            
* Output:           
* Notes:            
******************************************************************************/
int main(void)
{
	port_init();
	timer_init();
	lcd_init();
	external_int_init();
	uart0_init(BAUD9600);
	
	sei();
	/* Replace with your application code */
	while(1)
	{
		
		// CAN halozat kommunikacio
		if(task_10ms == TRUE)
		{
			// speed, pos kikuldese CAN-en
			task_10ms = FALSE;
		}
		
		
		//LCD megjelenites
		if(task_500ms == TRUE)
		{
			char write_string[50];
			sprintf(write_string,"pos:%d,%d pszi:%d",pose[0], pose[1], pose[3]);
			lcd_set_cursor_position(0);
			lcd_write_string(write_string);
			
			task_500ms = FALSE;
		}
		
		
		//UART kommunikacio
		if(task_100ms == TRUE)
		{
			char write_string[50];
			sprintf(write_string,"posx:%d posy:%d pszi:%d",pose[0], pose[1], pose[3]);
			uart_write_string(write_string);
			
			task_100ms = FALSE;
		}
	}
}


/******************************************************************************
* Interrupt Routines
******************************************************************************/
ISR(TIMER0_COMP_vect) //timer CTC interrupt
{
	timer_cnt++;
	if((timer_cnt % 1) == 0) task_10ms = TRUE;
	if((timer_cnt % 10) == 0) task_100ms =TRUE;
	if((timer_cnt % 50) == 0) task_500ms = TRUE;
}