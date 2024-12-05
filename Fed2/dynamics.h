#ifndef DYNAMICS_H_
#define DYNAMICS_H_
/******************************************************************************
* Include files
******************************************************************************/


/******************************************************************************
* Types
******************************************************************************/


/******************************************************************************
* Constants
******************************************************************************/


/******************************************************************************
* Macros
******************************************************************************/


/******************************************************************************
* Global Function Declarations
******************************************************************************/
double calculate_long_speed(double g_allas, double f_allas);
double calculate_lateral_speed(double delta);
void calculate_global_pose(void);
double posex(void);
double posey(void);
double pszit(void);




#endif /* DINAMICS_H_ */