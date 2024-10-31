/******************************************************************************
 * Created: 
 * Author : Neukum Csanád
******************************************************************************/
 /******************************************************************************
* Include files
******************************************************************************/
#include <inttypes.h>
#include <math.h>

/******************************************************************************
* Macros
******************************************************************************/


/******************************************************************************
* Constants
******************************************************************************/
double m = 1573;
double time = 0.0001;
/******************************************************************************
* Global Variables
******************************************************************************/
double v = 0;
double a = 0;
double pszi = 0;
double pszi_dot = 0;
double vy = 0;
/******************************************************************************
* External Variables
******************************************************************************/
double accel;
double decel;
double steer;

double pose[3];

/******************************************************************************
* Local Function Declarations
******************************************************************************/
double integrate(time, x_dot);
calculate_long_speed(g_allas, f_allas);
calculate_lateral_speed(delta);
double calculate_global_psoe(void);
/******************************************************************************
* Local Function Definitions
******************************************************************************/
/******************************************************************************
* Function:         float integrate(time, x_dot)
* Description:      Calculates the time based integral
* Input:            time - the time between signals; x_dot the derivates of x
* Output:           x
* Notes:            
******************************************************************************/
double integrate(time, x_dot)
{   
    int cnt = 0;
    double num = 0;
    double sum =+ x_dot;
    double x;
    if (num != x_dot)
    {
        cnt++;
        num = x_dot;
    }

    if(cnt == 0)
    {
        x = 0;
    }

    if(cnt == 1)
    {
        x = x_dot * time;
    }

    else
    {
        x = (time/(sizeof(num)))*sum;
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
calculate_long_speed(g_allas, f_allas)
{
    //maximális erők
    double F_vmax = 15000;
    double F_fmax = 15000;
    //aerodinamikai tényezők
    double rho = 1.2;
    double Cd = 0.208;
    double Af = 1.5;

    a = ((rho/2)*Cd*Af*v*v)/m + (F_vmax/m)*g_allas - (F_fmax/m)*f_allas;
    v = integrate(time, a);
}
/******************************************************************************
* Function:         calculate_lateral_speed(delta)
* Description:      oldalirányú sebesség számolása
* Input:            delta - kormányszög
* Output:           vy
* Notes:            
******************************************************************************/
calculate_lateral_speed(delta)
{
    int C1 = 80000;
    int C2 = 80000;
    double l1 = 1.1;
    double l2 = 1.58;
    int J = 2873;

    double pszi_2dot = (-(C1* pow(l1,2) - C2 * pow(l2, 2))/(J * v))*pszi_dot + (-(C1*l1 + C2*l2)/(J*v))*vy + ((C1*l1)/J)*delta;

    double vy_dot = ((-(C1*l1 + C2*l2)/(m*v))-v)*pszi_dot + (-(C1*C2)/(m*v))*vy + (C1/m)*delta;

    pszi_dot = integrate(time, pszi_2dot);

    vy = integrate(time, vy_dot);
}
/******************************************************************************
* Function:         double calculate_global_psoe(void)
* Description:      globális pozíció számolása
* Input:            void
* Output:           x, y - pozíció
* Notes:            
******************************************************************************/

double calculate_global_pose(void)
{
    double speed_x = v * cos(pszi) + vy * sin(pszi);
    double speed_y = v * sin(pszi) + vy * cos(pszi);
    pszi_dot = pszi_dot;

    double pose_x = integrate(time, speed_x);
    double pose_y = integrate(time, speed_y);
    pszi = integrate(time, pszi_dot);

    double pose[3] = {pose_x, pose_y, pszi};

    return pose[3];
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
	/* Replace with your application code */
	while(1)
	{
        v = calculate_long_speed(accel, decel);
        calculate_lateral_speed(steer);
        pose[3] = calculate_global_pose();
	}
}


/******************************************************************************
* Interrupt Routines
******************************************************************************/