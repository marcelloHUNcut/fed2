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
#include <stdio.h>

#define F_CPU 8000000UL
#include <avr/delay.h>

#include <inttypes.h>
#include "lcd.h"
#include "uart.h"
#include "peripherals.h"
#include "dynamics.h"
#include "can.h"

/******************************************************************************
* Macros
******************************************************************************/
#define TRUE 1
#define FALSE 0

#define PD0_ENA_DELAY 80

/******************************************************************************
* Constants
******************************************************************************/


/******************************************************************************
* Global Variables
******************************************************************************/
uint8_t task_10ms=FALSE, task_100ms=FALSE, task_500ms=FALSE, task_1s=FALSE;
uint16_t timer_cnt=0;

uint8_t PB0_pushed = 0, PD0_re_enable_cnt = 0;

//CAN recieve
uint8_t can_rx_data[8];
uint32_t can_rx_id = 0x11;
uint8_t can_rx_extended_id = FALSE;
uint8_t can_rx_length;
uint8_t can_msg_received = FALSE;

double steer_angle;
uint16_t throttle;
uint16_t brake;
uint8_t kick_down;
uint8_t cnt_3bit;

//CAN transmit
uint8_t can_tx_data[8];

int pos_x = 0;
int pos_y = 0;
uint16_t v_x = 0;
int v_y = 0;
int yaw = 0;
uint8_t cnt_2bit = 0;



//double accel_pos = 0;
//double decel_pos = 0;
//double steer_angle = 0;

uint8_t accel = 50;
uint8_t decel = 0;
double steer = 0;


/******************************************************************************
* External Variables
******************************************************************************/

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
	DDRA = 0xff;
	PORTA = 0x00;
	DDRF=0xff;
	
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
	can_init();
	CAN_ReceiveEnableMob(3, can_rx_id, can_rx_extended_id, 5);	// enable reception on mob 0
	
	sei();
	/* Replace with your application code */
	while(1)
	{
		
		// CAN halozat kommunikacio
		if(task_10ms == TRUE)
		{	
			if(can_msg_received)
			{
				steer_angle = (can_rx_data[0] | ((int)(can_rx_data[1]) << 8))*0.04;
				throttle = can_rx_data[2] | (int)(((can_rx_data[3] & 0b00000011) << 8));
				brake = (can_rx_data[3] >> 2) | (int)(((can_rx_data[4] & 0b00001111) << 6));
				kick_down = (((can_rx_data[4] & 0b00010000) >> 4));
				cnt_3bit = can_rx_data[4] >> 5;
				can_msg_received=0;
			}
			
			pos_x = (int)posex();
			pos_y = (int)posey();
			v_x = calculate_long_speed(throttle, brake);
			v_y = calculate_lateral_speed(steer_angle*(M_PI/180.0));
			yaw = (int)pszit();
			
			uint8_t can_tx_data[8];
			can_tx_data[0] = pos_x;
			can_tx_data[1] = (pos_x >> 8) | (pos_y << 4);
			can_tx_data[2] = pos_y >> 4;
			can_tx_data[3] = v_x;
			can_tx_data[4] = (v_x >> 8) | (v_y << 4);
			can_tx_data[5] = (v_y >> 4) | (yaw << 6);
			can_tx_data[6] = (yaw >> 2);
			can_tx_data[7] = (yaw >> 10) | (cnt_2bit << 6);
			cnt_2bit++;
			
			CAN_SendMob(2, 0x22, FALSE, 8, can_tx_data);
			
			task_10ms = FALSE;
		}
		
		
		//LCD megjelenites
		if(task_500ms == TRUE)
		{
			/*pos_x = pos_x+1;
			pos_y = pos_y-1;
			v_x = 657;
			v_y = 453;
			yaw = 370;*/
			
			char write_string[50];
			sprintf(write_string,"pos:%5d,%5d", pos_x, pos_y);
			lcd_set_cursor_position(0);
			lcd_write_string(write_string);
			sprintf(write_string,"pszi:%4d", (yaw));
			lcd_set_cursor_position(40);
			lcd_write_string(write_string);
			
			task_500ms = FALSE;
		}
		
		
		//UART kommunikacio
		if(task_100ms == TRUE)
		{
			 uart_write(0xF0);  // Start byte

			 uint16_t x_value = (uint16_t)pos_x;
			 uint16_t y_value = (uint16_t)pos_y;
			 uint16_t pszi_value = (uint16_t)yaw;

			 uart_write((uint8_t)(x_value & 0xFF));           // x_low
			 uart_write((uint8_t)((x_value >> 8) & 0xFF));    // x_high

			 uart_write((uint8_t)(y_value & 0xFF));           // y_low
			 uart_write((uint8_t)((y_value >> 8) & 0xFF));    // y_high

			 uart_write((uint8_t)(pszi_value & 0xFF));        // pszi_low
			 uart_write((uint8_t)((pszi_value >> 8) & 0xFF)); // pszi_high

			 uart_write(0xFF);  // End byte

			 task_100ms = FALSE;
		}
		
		if(task_1s == TRUE)
		{
			PORTF ^= (1<<PF3);
			task_1s = FALSE;
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
	if((timer_cnt % 100) == 0) task_1s = TRUE;
}

ISR(INT0_vect) //external interrupt
{
	if(PD0_re_enable_cnt == PD0_ENA_DELAY) //pergésmentesítés logika
	{
		PORTA ^=0x02;
		PD0_re_enable_cnt=0;
	}
}

ISR(CANIT_vect) //CAN megszakítás
{
	uint8_t i, dlc = 0;
	
	CANPAGE = 3<<4;	// select MOb0, reset FIFO index

	if ( (CANSTMOB & (1<<RXOK)) != FALSE)	// Receive Complete
	{
		
		dlc = CANCDMOB & 0x0F;
		
		for (i=0; i<dlc; i++) can_rx_data[i] = CANMSG;
		
		CANSTMOB &= ~(1<<RXOK);	// clear RXOK flag
		CAN_ReceiveEnableMob(3, can_rx_id, can_rx_extended_id, 5);	// enable next reception  on mob 0
	}
	can_rx_length=dlc;
	can_msg_received=1;
	PORTA = PORTA ^ (1<<PA7);
}