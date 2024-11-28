/******************************************************************************
 * Created: 
 * Author : Neukum Csanád
******************************************************************************/
 /******************************************************************************
* Include files
******************************************************************************/
#include <inttypes.h>
#include <math.h>
#include "dynamics.h"

/******************************************************************************
* Macros
******************************************************************************/


/******************************************************************************
* Constants
******************************************************************************/
uint32_t m = 1573;
//uint32_t ido = 0.0001;
/******************************************************************************
* Global Variables
******************************************************************************/
uint32_t v = 0;
uint32_t sum_a = 0;
uint8_t cnt_a = 0;

uint32_t pszi = 0;
uint32_t pszi_dot = 0;
uint32_t pszi_2dot = 0;
int cnt_pszi_2dot = 0;
uint32_t vy = 0;
uint32_t vy_dot = 0;
int cnt_vy_dot = 0;

uint32_t speed_x = 0;
int cnt_x = 0;
uint32_t speed_y = 0;
int cnt_y = 0;
uint32_t sum_pszi_dot = 0;
int cnt_pszi_dot = 0;

uint32_t sum_vy_dot = 0;

/******************************************************************************
* External Variables
******************************************************************************/
uint32_t accel;
uint32_t decel;
uint32_t steer;

uint32_t pose[3];

/******************************************************************************
* Local Function Declarations
******************************************************************************/
uint32_t integrate(uint32_t x_dot, uint32_t sum, uint8_t cnt);

/******************************************************************************
* Local Function Definitions
******************************************************************************/
/******************************************************************************
* Function:         uint32_t integrate(ido, x_dot)
* Description:      Calculates the time based integral
* Input:            time - the time between signals; x_dot the derivates of x
* Output:           x
* Notes:            
******************************************************************************/
uint32_t integrate(uint32_t x_dot, uint32_t sum, uint8_t cnt)
{   
    uint32_t x;
	uint8_t ido = 5;

    if(cnt == 0)
    {
        x = 0;
    }

    if(cnt == 1)
    {
        x = x_dot * (ido/10);
    }
    else
    {
        x = ((ido/(10))/(cnt))*sum;
    }
    
    return x;
}
/******************************************************************************
* Function:         calculate_long_speed(g_allas, f_allas, v)
* Description:      hosszirányú sebesség számolása
* Input:            gázpedál állás, fékpedál állás, jelenlegi hosszirányú sebesség
* Output:           új hosszirányú sebesség
* Notes:            
******************************************************************************/
uint32_t calculate_long_speed(uint32_t g_allas, uint32_t f_allas)
{
    //maximális erők
    uint16_t F_vmax = 15000;
    uint16_t F_fmax = 15000;
    //aerodinamikai tényezők
    uint8_t rho = 12;
    uint16_t Cd = 208;
    uint8_t Af = 15;
	
    uint32_t a_new = ((rho/20)*(Cd/1000)*(Af/10)*v*v)/m + (F_vmax/m)*(g_allas/100) - (F_fmax/m)*(f_allas/100);

    sum_a += a_new;
    cnt_a++;
    v = integrate(a_new, sum_a, cnt_a);
	
	return v;
}
/******************************************************************************
* Function:         calculate_lateral_speed(delta)
* Description:      oldalirányú sebesség számolása
* Input:            delta - kormányszög
* Output:           vy
* Notes:            
******************************************************************************/
void calculate_lateral_speed(uint32_t delta)
{
    int C1 = 80000;
    int C2 = 80000;
    uint32_t l1 = 1.1;
    uint32_t l2 = 1.58;
    int J = 2873;

    uint32_t pszi_2dot_new = (-(C1* pow(l1,2) - C2 * pow(l2, 2))/(J * v))*pszi_dot + (-(C1*l1 + C2*l2)/(J*v))*vy + ((C1*l1)/J)*delta;

    uint32_t vy_dot_new = ((-(C1*l1 + C2*l2)/(m*v))-v)*pszi_dot + (-(C1*C2)/(m*v))*vy + (C1/m)*delta;

    
    uint32_t sum_2pszi_dot = pszi_2dot + pszi_2dot_new;
    cnt_pszi_2dot++;
    pszi_dot = integrate(pszi_2dot_new, (uint8_t)sum_2pszi_dot, cnt_pszi_2dot);
    pszi_2dot = pszi_2dot_new;

    sum_vy_dot = vy_dot + vy_dot_new;
    cnt_vy_dot++;
    vy = integrate(vy_dot, (uint8_t)sum_vy_dot, cnt_vy_dot);
    vy_dot = vy_dot_new;
}
/******************************************************************************
* Function:         uint32_t calculate_global_psoe(void)
* Description:      globális pozíció számolása
* Input:            void
* Output:           x, y - pozíció
* Notes:            
******************************************************************************/

uint32_t calculate_global_x(void)
{
    uint32_t speed_x_new = v * cos(pszi) + vy * sin(pszi);

    uint32_t sum_x = speed_x + speed_x_new;
    cnt_x++;
    uint32_t pose_x = integrate(speed_x_new, (uint8_t)sum_x, cnt_x);
    speed_x = speed_x_new;

    return pose_x;
}

uint32_t calculate_global_y(void)
{
	uint32_t speed_y_new = v * sin(pszi) + vy * cos(pszi);

	uint32_t sum_y = speed_y + speed_y_new;
	cnt_y++;
	uint32_t pose_y = integrate(speed_y_new, (uint8_t)sum_y, cnt_y);
	speed_y = speed_y_new;

	return pose_y;
}

uint32_t calculate_global_pszi(void)
{
	pszi_dot = pszi_dot;

	sum_pszi_dot = sum_pszi_dot + pszi_dot;
	cnt_pszi_dot++;
	pszi = integrate(pszi_dot, (uint8_t)sum_pszi_dot, cnt_pszi_dot);

	return pszi;
}

/******************************************************************************
* Function:         int main(void)
* Description:      main function
* Input:            
* Output:           
* Notes:            
******************************************************************************/


/******************************************************************************
* Interrupt Routines
******************************************************************************/