#include "software_callback_function.h"
#include "quadrangle.h"
#include "PID.h"
#include "Stepper.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "sys.h"

TimerHandle_t sendto_Upper_Handle = NULL; //+发送数据到上位机定时器句柄
TimerHandle_t motor_running_Handle = NULL; //+巡线PID定时器句柄



// extern variables
extern struct PID* X_PID_handle;
extern struct PID* Y_PID_handle;

extern struct Steeper_t* red_x_stepper_motor_handle;
extern struct Steeper_t* red_y_stepper_motor_handle;
extern struct quadrangle_t* quadrangle_handle;

extern EventGroupHandle_t Group_One_Handle; //+事件组句柄

extern struct dot_t* dot_red_handle;
extern struct dot_t* dot_green_handle;

/**
 * @description: this function is the software callback function that send data to upper computer
 * @return {*}
 */
void sendto_Upper(void)
{
}


int32_t x_target = 0;
int32_t y_target = 0;
/**
 * @description: this function is the software callback function that update the stepper motor position
 * @return {*}
 */
void motor_running(void)
{
    
}




