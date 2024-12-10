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
#include "dynamics.h"

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

uint8_t accel = 50;
uint8_t decel = 0;
double steer = 0;

uint16_t v_long = 0;
uint16_t x, y, pszi_global = 0;

/******************************************************************************
* External Variables
******************************************************************************/
//teszteles
//typedef union{
	//uint8_t BYTES[3];
	//struct
	//{
		//uint8_t x = (uint8_t)pose[0];
		//uint8_t y = (uint8_t)pose[1];
		//uint8_t yaw = (uint8_t)pose[2];
		//
	//} message;
//} stecc_message_t;

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
	
	// UART küldési függvény (el?feltételezve, hogy van UART író függvény)
	void uart_write(uint8_t data) {
		// Implementáljuk a tényleges UART küldést itt.
		// Ez a függvény elküldi a `data` byte-ot az UART-on keresztül.
		while (!(UCSR0A & (1 << UDRE0))); // Várakozás, amíg az adó puffer üres
		UDR0 = data; // Küldjük el az adatot
	}
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
			
			v_long = calculate_long_speed(accel, decel);
			calculate_lateral_speed(steer);
			x = calculate_global_x();
			y = calculate_global_y();
			pszi_global = calculate_global_pszi();
			
			
			char write_string[50];
			sprintf(write_string,"pos:%3d,%3d", (uint8_t)v_long, (uint8_t)y);
			lcd_set_cursor_position(0);
			lcd_write_string(write_string);
			sprintf(write_string,"pszi:%3d", (uint8_t)(pszi_global*(180/M_PI)));
			lcd_set_cursor_position(40);
			lcd_write_string(write_string);
			
			
			task_500ms = FALSE;
		}
		
		
		//UART kommunikacio
		if(task_100ms == TRUE)
		{
			 uint8_t x_pos_low = (uint8_t)x; // Alacsony byte
			 uint8_t x_pos_high = (uint8_t)(x >> 8); // Magas byte
			 uint8_t y_pos_low = (uint8_t)y; // Alacsony byte
			 uint8_t y_pos_high = (uint8_t)(y >> 8); // Magas byte
			 uint8_t pszi_global_low = (uint8_t)pszi_global; // Alacsony byte
			 uint8_t pszi_global_high = (uint8_t)(pszi_global >> 8); // Magas byte
			 
			 // Checksum kiszámítása
			 uint8_t checksum = 0xAA ^ x_pos_low ^ x_pos_high ^ y_pos_low ^ y_pos_high ^ pszi_global_low ^ pszi_global_high;
			 
			 // Üzenet küldése
			 uart_write(0xAA);             // Start byte
			 uart_write(x_pos_low);        // x_pos alsó byte
			 uart_write(x_pos_high);       // x_pos fels? byte
			 uart_write(y_pos_low);        // y_pos alsó byte
			 uart_write(y_pos_high);       // y_pos fels? byte
			 uart_write(pszi_global_low);  // pszi_global alsó byte
			 uart_write(pszi_global_high); // pszi_global fels? byte
			 uart_write(checksum);         // Checksum küldése
			 
			 task_100ms = FALSE;           // Reseteljük a flag-et
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
