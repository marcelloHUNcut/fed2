/******************************************************************************
* Include files
******************************************************************************/
#include "peripherals.h"
#include <avr/io.h>
/******************************************************************************
* Macros
******************************************************************************/


/******************************************************************************
* Constants
******************************************************************************/


/******************************************************************************
* Global Variables
******************************************************************************/


/******************************************************************************
* External Variables
******************************************************************************/


/******************************************************************************
* Local Function Declarations
******************************************************************************/


/******************************************************************************
* Local Function Definitions
******************************************************************************/


/******************************************************************************
* Global Function Definitions
******************************************************************************/
/******************************************************************************
* Function:         void timer_init(void)
* Description:      timer inicializalasa
* Input:
* Output:
* Notes:			10 ms, CTC
******************************************************************************/
void timer_init(void)
{
	TCCR0A = (0<<WGM00) | (1<<WGM01) | (1<<CS02) | (0<<CS01) | (1<<CS00);
	OCR0A = 77;
	TIMSK0 = (1<<OCIE0A);
}
/******************************************************************************
* Function:         void external_int_init(void)
* Description:      kulso megszakatis beallitasa
* Input:
* Output:
* Notes:
******************************************************************************/
void external_int_init(void)
{
	EICRA = (1<<ISC01) | (0<<ISC00);
	EIMSK = (1<<INT0);
}

/******************************************************************************
* Function:         void adc_init(void)
* Description:      AD konverter felkonfiguralasa
* Input:
* Output:
* Notes:
******************************************************************************/
void adc_init(void)
{
	ADMUX = 0;	//PD0-as lab
	//ADMUX = (1 << MUX0);	//PD1-es lab
	//ADMUX = (1<<MUX1); //PD2-es lab
	//ADMUX = (1<<MUX1) | (1<<MUX0); //PD3-as lab
	//ADMUX = (1<<MUX2); //PD4-es lab
	ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}