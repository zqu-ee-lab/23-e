#ifndef SOFTWARE_CALLBACK_FUNCTION_H_
#define SOFTWARE_CALLBACK_FUNCTION_H_


#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>


extern TimerHandle_t sendto_Upper_Handle; //+发送数据到上位机定时器句柄
extern TimerHandle_t motor_running_Handle; //+巡线PID定时器句柄



void motor_running(void);
void sendto_Upper(void);            /* 用于初始化板载相关资源 */



#endif /* SOFTWARE_CALLBACK_FUNCTION_H_ */