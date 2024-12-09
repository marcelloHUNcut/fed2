/******************************************************************************
 * Created:
 * Author :
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
int m = 1573;
double time = 0.5;
/******************************************************************************
* Global Variables
******************************************************************************/
double sum_a = 0;
double sum_pszi2dot = 0;
double sum_vydot = 0;
double sum_speedx = 0;
double sum_speedy = 0;
double sum_pszidot = 0;

double v = 0;
double a = 0;
double pszi = 0;
double pszi_dot = 0;
double pszi_2dot = 0;
double speed_x = 0;
double speed_y = 0;
double vy = 0;
double vy_dot = 0;

//main consts
double steer_angle = 10;
uint16_t throttle = 50;
uint16_t brake = 0;

int pos_x = 0;
int pos_y = 0;
uint16_t v_x = 0;
int v_y = 0;
int yaw = 0;

/******************************************************************************
* External Variables
******************************************************************************/
//double accel;
//double decel;
//double steer;

double pose_x;
double pose_y;

/******************************************************************************
* Local Function Declarations
******************************************************************************/
double integrate(double time, double x_dot);
double calculate_long_speed(double g_allas, double f_allas);
double calculate_lateral_speed(double delta);
void calculate_global_pose(void);
double posex(void);
double posey(void);
double pszit(void);

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
double integrate(double time, double x_dot)
{
	int cnt = 0;
	double num = 0;
	double sum = +x_dot;
	double x;
	if (num != x_dot)
	{
		cnt++;
		num = x_dot;
	}

	if (cnt == 0)
	{
		x = 0;
	}

	if (cnt == 1)
	{
		x = x_dot * (time);
	}

	else
	{
		x = ((time)*sum);
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
double calculate_long_speed(double g_allas, double f_allas)
{
	//maximális erõk
	double F_vmax = 10000;
	double F_fmax = 10000;
	//aerodinamikai tényezõk
	double rho = 0.12;
	double Cd = 0.208;
	double Af = 1.5;

	a = ((((rho / 2) * (Cd) * (Af)*v * v) / m) + (F_vmax / m) * (g_allas/100)-(F_fmax / m) * (f_allas/100));
	sum_a = sum_a + a;
	if (v > (180 / 3.6))
	{
		v = 180 / 3.6;
	}
	else
	{
		v = integrate(time, sum_a);
	}
	return v;
}
/******************************************************************************
* Function:         calculate_lateral_speed(delta)
* Description:      oldalirányú sebesség számolása
* Input:            delta - kormányszög
* Output:           vy
* Notes:
******************************************************************************/
double calculate_lateral_speed(double delta)
{
	double C1 = 16000;
	double C2 = 16000;
	double l1 = 1.1;
	double l2 = 1.58;
	double J = 2873;
	if (v == 0 || delta == 0)
	{
		pszi_2dot = 0;
		vy_dot = 0;
	}
	else
	{
		pszi_2dot = (((-C1 * (l1 * l1) - C2 * (l2 * l2)) / (J * v)) * pszi_dot) + (((-C1 * l1 + C2 * l2) / (J * v)) * vy) + (((C1 * l1) / J) * delta);
		vy_dot = ((((-C1 * l1 + C2 * l2) / (m * v)) - v) * pszi_dot) + (((-C1 - C2) / (m * v)) * vy) + ((C1 / m) * delta);
	}

	sum_pszi2dot = sum_pszi2dot + pszi_2dot;
	sum_vydot = sum_vydot + vy_dot;

	pszi_dot = integrate(time, sum_pszi2dot);
	vy = integrate(time, sum_vydot);
	return vy;
}
/******************************************************************************
* Function:         double calculate_global_psoe(void)
* Description:      globális pozíció számolása
* Input:            void
* Output:           x, y - pozíció
* Notes:
******************************************************************************/

void calculate_global_pose(void)
{
	speed_x = (v)*cos(pszi) + vy * sin(pszi);
	speed_y = (v)*sin(pszi) + vy * cos(pszi);
	pszi_dot = pszi_dot;

	sum_speedx = sum_speedx + speed_x;
	sum_speedy = sum_speedy + speed_y;
	sum_pszidot = sum_pszidot + pszi_dot;

	pose_x = integrate(time, sum_speedx);
	pose_y = integrate(time, sum_speedy);
	pszi = integrate(time, sum_pszidot);

}
double posex(void)
{
	return pose_x;
}
double posey(void)
{
	return pose_y;
}
double pszit(void)
{
	return pszi;
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
	while (1)
	{
		
		v_x = (uint16_t)calculate_long_speed(throttle, brake);
		v_y = (int)calculate_lateral_speed(steer_angle * (3.1415 / 180.0));
		calculate_global_pose();
		pos_x = (int)posex();
		pos_y = (int)posey();
		yaw = (int)pszit();
	}
}


/******************************************************************************
* Interrupt Routines
******************************************************************************/