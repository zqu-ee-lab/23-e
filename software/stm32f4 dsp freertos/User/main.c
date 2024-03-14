/*
 * ......................................&&.........................
 * ....................................&&&..........................
 * .................................&&&&............................
 * ...............................&&&&..............................
 * .............................&&&&&&..............................
 * ...........................&&&&&&....&&&..&&&&&&&&&&&&&&&........
 * ..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............
 * ................&...&&&&&&&&&&&&&&&&&&&&&&&&&&&&.................
 * .......................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........
 * ...................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...............
 * ..................&&&   &&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * ...............&&&&&@  &&&&&&&&&&..&&&&&&&&&&&&&&&&&&&...........
 * ..............&&&&&&&&&&&&&&&.&&....&&&&&&&&&&&&&..&&&&&.........
 * ..........&&&&&&&&&&&&&&&&&&...&.....&&&&&&&&&&&&&...&&&&........
 * ........&&&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&....&&&.......
 * .......&&&&&&&&.....................&&&&&&&&&&&&&&&&.....&&......
 * ........&&&&&.....................&&&&&&&&&&&&&&&&&&.............
 * ..........&...................&&&&&&&&&&&&&&&&&&&&&&&............
 * ................&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&............
 * ..................&&&&&&&&&&&&&&&&&&&&&&&&&&&&..&&&&&............
 * ..............&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&....&&&&&............
 * ...........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&......&&&&............
 * .........&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.........&&&&............
 * .......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&............
 * ......&&&&&&&&&&&&&&&&&&&...&&&&&&...............&&&.............
 * .....&&&&&&&&&&&&&&&&............................&&..............
 * ....&&&&&&&&&&&&&&&.................&&...........................
 * ...&&&&&&&&&&&&&&&.....................&&&&......................
 * ...&&&&&&&&&&.&&&........................&&&&&...................
 * ..&&&&&&&&&&&..&&..........................&&&&&&&...............
 * ..&&&&&&&&&&&&...&............&&&.....&&&&...&&&&&&&.............
 * ..&&&&&&&&&&&&&.................&&&.....&&&&&&&&&&&&&&...........
 * ..&&&&&&&&&&&&&&&&..............&&&&&&&&&&&&&&&&&&&&&&&&.........
 * ..&&.&&&&&&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&&&&&&&&&&&&.......
 * ...&&..&&&&&&&&&&&&.........&&&&&&&&&&&&&&&&...&&&&&&&&&&&&......
 * ....&..&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&...........&&&&&&&&.....
 * .......&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&..............&&&&&&&....
 * .......&&&&&.&&&&&&&&&&&&&&&&&&..&&&&&&&&...&..........&&&&&&....
 * ........&&&.....&&&&&&&&&&&&&.....&&&&&&&&&&...........&..&&&&...
 * .......&&&........&&&.&&&&&&&&&.....&&&&&.................&&&&...
 * .......&&&...............&&&&&&&.......&&&&&&&&............&&&...
 * ........&&...................&&&&&&.........................&&&..
 * .........&.....................&&&&........................&&....
 * ...............................&&&.......................&&......
 * ................................&&......................&&.......
 * .................................&&..............................
 * ..................................&..............................
 *
 * @Author: zhaojianchao jar-chou 2722642511@qq.com
 * @Date: 2023-09-06 13:02:19
 * @LastEditors: jar-chou 2722642511@qq.com
 * @LastEditTime: 2023-09-07 13:50:25
 * @FilePath: \delivering_car\User\main.c
 * @Description: 龙王保佑此文件无bug！！！
 */

/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "queue.h"
/* 开发板硬件bsp头文件 */
#include "bsp_led.h"
#include "usart.h"
#include "delay.h"
#include "buffer.h"
#include "PID.h"
#include "stdio.h"
#include "OLED.h"
#include "sys.h"
#include "key.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "encoder.h"

// user header file
#include "quadrangle.h"
#include "Stepper.h"
#include "SysInfoTest.h"
#include "my_math.h"
#include "oled_draw.h"
#include "oled_buffer.h"

/**************************** 任务句柄 ********************************/
/*
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */

#define pulse_per_pixel 0.091
#define pulse_per_pixel_green 0.138

// const float32_t pulse_per_pixel_red_x[2] = {0.085, 0.005};
// const float32_t pulse_per_pixel_red_y[2] = {0.0025, 0.085};
// const float32_t pulse_per_pixel_green_x[2] = {0.108, 0.005};
// const float32_t pulse_per_pixel_green_y[2] = {0.005, 0.108};

QueueHandle_t Task_Number_Handle = NULL;

static TaskHandle_t analyse_data_Handle = NULL;	 //+解析数据定时器句柄
static TaskHandle_t OLED_SHOW_Handle = NULL;	 //+OLDE显示句柄
static TaskHandle_t AppTaskCreate_Handle = NULL; //+创建任务句柄
static TaskHandle_t KEY_SCAN_Handle = NULL;		 //+KEY_SCAN句柄
static TaskHandle_t Task_schedule_Handle = NULL; //+SysInfoTestSent句柄
static TaskHandle_t Follow_pencil_line_Handle = NULL;
static TaskHandle_t Follow_By_Quadrangle_Handle = NULL;
static TaskHandle_t Return_The_Original_Point_Handle = NULL;
static TaskHandle_t Follow_Red_dot_Handle = NULL;

EventGroupHandle_t Group_One_Handle = NULL; //+事件组句柄

/******************************* Global variable declaration ************************************/
/*
 * 当我们在写应用程序的时候，可能需要用到一些全局变量。
 */

struct Steeper_t *red_x_stepper_motor_handle = NULL;
struct Steeper_t *red_y_stepper_motor_handle = NULL;
struct Steeper_t *green_x_stepper_motor_handle = NULL;
struct Steeper_t *green_y_stepper_motor_handle = NULL;

struct quadrangle_t *quadrangle_handle = NULL;

struct dot_t dot_red_from_other_camera = {0, 0};
const struct dot_t dot_red_real = {.x = 325, .y = 224};
// struct dot_t dot_green;
// const struct dot_t dot_green_real = {.x = 278, .y = 21};

struct dot_t pencil_line[4] = {
	{.x = 19, .y = 271},
	{.x = 27, .y = 21},
	{.x = 285, .y = 271},
	{.x = 278, .y = 21},
};

int pos_red_x_motor = 0;
int pos_red_y_motor = 0;

struct PID X_PID;
struct PID Y_PID;
struct PID Green_X_PID;
struct PID Green_Y_PID;

struct PID *X_PID_handle = &X_PID;
struct PID *Y_PID_handle = &Y_PID;
struct PID *Green_X_PID_handle = &Green_X_PID;
struct PID *Green_Y_PID_handle = &Green_Y_PID;

static float32_t Red_Dot_X_Estimated_Error = 0;
static float32_t Red_Dot_Y_Estimated_Error = 0;

static float32_t Green_Dot_X_Estimated_Error = 0;
static float32_t Green_Dot_Y_Estimated_Error = 0;

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void Return_The_Original_Point(void *pvParameters);
static void analyse_data(void);
static void Follow_pencil_line(void *pvParameters);
static void Follow_By_Quadrangle(void *pvParameters);
static void AppTaskCreate(void);			   /* 用于创建任务 */
static void Task__TWO(void *pvParameters);	   /* Test_Task任务实现 */
static void OLED_SHOW(void *pvParameters);	   /* Test_Task任务实现 */
static void KEY_SCAN(void *pvParameters);	   /* Test_Task任务实现 */
static void Task_schedule(void *pvParameters); /* 任务调度任务实现 */
static void Follow_Red_dot(void *pvParameters);
static void BSP_Init(void); /* 用于初始化板载相关资源 */
static void USER_Init(void);

/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化
			第二步：创建APP应用任务
			第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
int main(void)
{
	BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */

	/* 开发板硬件初始化 */
	BSP_Init();

	USER_Init();

	USART1_DMA_TranData((uint8_t *)"hello world\r\n", 13);

	/* 创建AppTaskCreate任务 */
	xReturn = xTaskCreate((TaskFunction_t)AppTaskCreate,		  /* 任务入口函数 */
						  (const char *)"AppTaskCreate",		  /* 任务名字 */
						  (uint16_t)1024,						  /* 任务栈大小 */
						  (void *)NULL,							  /* 任务入口函数参数 */
						  (UBaseType_t)1,						  /* 任务的优先级 */
						  (TaskHandle_t *)&AppTaskCreate_Handle); /* 任务控制块指针 */
	/* 启动任务调度 */
	if (pdPASS == xReturn)
		vTaskStartScheduler(); /* 启动任务，开启调度 */
	else
		return -1;

	while (1)
		; /* 正常不会执行到这里 */
}

/***********************************************************************
 * @ 函数名  ： AppTaskCreate
 * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
 * @ 参数    ： 无
 * @ 返回值  ： 无
 **********************************************************************/
static void AppTaskCreate(void)
{
	BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为pdPASS */

	taskENTER_CRITICAL(); // 进入临界区

	/* 创建Test_Task任务 */

#if configGENERATE_RUN_TIME_STATS && configUSE_STATS_FORMATTING_FUNCTIONS
	xReturn = xTaskCreate((TaskFunction_t)SysInfoTestSent,			/* 任务入口函数 */
						  (const char *)"SysInfoTestSent",			/* 任务名字 */
						  (uint16_t)512,							/* 任务栈大小 */
						  (void *)NULL,								/* 任务入口函数参数 */
						  (UBaseType_t)1,							/* 任务的优先级 */
						  (TaskHandle_t *)&SysInfoTestSent_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		printf("SysInfoTestSent任务创建成功\r\n");
#endif
	xReturn = xTaskCreate((TaskFunction_t)KEY_SCAN,			 /* 任务入口函数 */
						  (const char *)"KEY_SCAN",			 /* 任务名字 */
						  (uint16_t)512,					 /* 任务栈大小 */
						  (void *)NULL,						 /* 任务入口函数参数 */
						  (UBaseType_t)10,					 /* 任务的优先级 */
						  (TaskHandle_t *)&KEY_SCAN_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		App_Printf("KEY_SCAN任务创建成功\r\n");
	xReturn = xTaskCreate((TaskFunction_t)OLED_SHOW,		  /* 任务入口函数 */
						  (const char *)"OLED_SHOW",		  /* 任务名字 */
						  (uint16_t)512,					  /* 任务栈大小 */
						  (void *)NULL,						  /* 任务入口函数参数 */
						  (UBaseType_t)3,					  /* 任务的优先级 */
						  (TaskHandle_t *)&OLED_SHOW_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		App_Printf("OLED_SHOW任务创建成功\r\n");
	xReturn = xTaskCreate((TaskFunction_t)analyse_data,
						  (const char *)"analyse_data",
						  (uint16_t)256,						 /* 任务栈大小 */
						  (void *)NULL,							 /* 任务入口函数参数 */
						  (UBaseType_t)7,						 /* 任务的优先级 */
						  (TaskHandle_t *)&analyse_data_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		App_Printf("analyse_data任务创建成功\r\n");
	xReturn = xTaskCreate((TaskFunction_t)Task_schedule,
						  (const char *)"Task_schedule",
						  (uint16_t)256,						  /* 任务栈大小 */
						  (void *)NULL,							  /* 任务入口函数参数 */
						  (UBaseType_t)5,						  /* 任务的优先级 */
						  (TaskHandle_t *)&Task_schedule_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		App_Printf("Task_schedule任务创建成功\r\n");
	xReturn = xTaskCreate((TaskFunction_t)Follow_pencil_line,
						  (const char *)"Follow_pencil_line",
						  (uint16_t)256,							   /* 任务栈大小 */
						  (void *)NULL,								   /* 任务入口函数参数 */
						  (UBaseType_t)4,							   /* 任务的优先级 */
						  (TaskHandle_t *)&Follow_pencil_line_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		App_Printf("Follow_pencil_line任务创建成功\r\n");
	xReturn = xTaskCreate((TaskFunction_t)Follow_By_Quadrangle,
						  (const char *)"Follow_By_Quadrangle",
						  (uint16_t)256,								 /* 任务栈大小 */
						  (void *)NULL,									 /* 任务入口函数参数 */
						  (UBaseType_t)4,								 /* 任务的优先级 */
						  (TaskHandle_t *)&Follow_By_Quadrangle_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		App_Printf("Follow_By_Quadrangle任务创建成功\r\n");
	xReturn = xTaskCreate((TaskFunction_t)Return_The_Original_Point,
						  (const char *)"Return_The_Original_Point",
						  (uint16_t)256,									  /* 任务栈大小 */
						  (void *)NULL,										  /* 任务入口函数参数 */
						  (UBaseType_t)4,									  /* 任务的优先级 */
						  (TaskHandle_t *)&Return_The_Original_Point_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		App_Printf("Return_The_Original_Point任务创建成功\r\n");
	xReturn = xTaskCreate((TaskFunction_t)Follow_Red_dot,
						  (const char *)"Follow_Red_dot",
						  (uint16_t)256,						   /* 任务栈大小 */
						  (void *)NULL,							   /* 任务入口函数参数 */
						  (UBaseType_t)4,						   /* 任务的优先级 */
						  (TaskHandle_t *)&Follow_Red_dot_Handle); /* 任务控制块指针 */
	if (xReturn == pdPASS)
		App_Printf("Follow_Red_dot任务创建成功\r\n");

	Task_Number_Handle = xQueueCreate(1, 1); // 开始解析数据
	Group_One_Handle = xEventGroupCreate();
	SET_EVENT(GAME_OVER);

	// 挂机任务，等待选择任务
	vTaskSuspend(Follow_pencil_line_Handle);
	vTaskSuspend(Follow_By_Quadrangle_Handle);
	vTaskSuspend(Return_The_Original_Point_Handle);
	vTaskSuspend(Follow_Red_dot_Handle);
	// vTaskSuspend(Task__TWO_Handle);
	vTaskDelete(AppTaskCreate_Handle); // 删除AppTaskCreate任务

	taskEXIT_CRITICAL(); // 退出临界区
}

/**
 * @description: this function is the software callback function that analyse data
 * @return {*}
 */
static void analyse_data(void)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(15); // 10ms
	xLastWakeTime = xTaskGetTickCount();
	const char head_red_laser[2] = {0xFE, 0xFE};
	const char head_green_laser[2] = {0xFE, 0xFD};
	const char head_quadrangle[2] = {0xFE, 0xFF};

	static int pos_green_x_local = 0;
	static int pos_green_y_local = 0;
	static int pos_red_x_local = 0;
	static int pos_red_y_local = 0;
	static int time_s = 0;
	struct quadrangle_t *quadrangle_local_handle = Quadrangle_Init();
	struct dot_t dot_red_local;
	struct dot_t dot_green_local;
	bool get_qu = true;
	char str[35];
	for (;;)
	{
		/* code */

		// usart2 get data
		// get the quadrangle data
		if (BUFF_pop_with_check_by_Protocol(U5_buffer_handle, head_quadrangle, 2, quadrangle_local_handle->dots, 16, 1, 8) == 8)
		{
			// App_Printf("get quadrangle data\r\n");
			// DrawString(1, 0, "get_quadrangle");
			quadrangle_local_handle->Sort(quadrangle_local_handle);
			quadrangle_local_handle->Equal_Scaling(quadrangle_local_handle, 0.91);
			// quadrangle_local_handle->GetDotsOnLines(quadrangle_local_handle);
			memcpy(quadrangle_handle, quadrangle_local_handle, sizeof(struct quadrangle_t));
			// App_Printf("quadrangle data is:\n%d %d \n%d %d\n%d %d \n%d %d\n", quadrangle_handle->dots[0].x, quadrangle_handle->dots[0].y,
			// 		   quadrangle_handle->dots[1].x, quadrangle_handle->dots[1].y, quadrangle_handle->dots[2].x, quadrangle_handle->dots[2].y,
			// 		   quadrangle_handle->dots[3].x, quadrangle_handle->dots[3].y);
			Red_Dot_X_Estimated_Error = 0;
			Red_Dot_Y_Estimated_Error = 0;
			SET_EVENT(GOT_QUADRANGLE);
		}
		else
		{
			DrawString(1, 0, "               ");
		}

		// get the red laser dot
		if (BUFF_pop_with_check_by_Protocol(U5_buffer_handle, head_red_laser, 2, &dot_red_local, 16, 1, 2) == 2)
		{
			// App_Printf("get laser data\r\n");
			// App_Printf("red dot is:%d %d\n", dot_red_local.x, dot_red_local.y);
			if ((dot_red_from_other_camera.x) + (dot_red_from_other_camera.y) == 0 || (abs(dot_red_local.x - dot_red_from_other_camera.x) < 70 && abs(dot_red_local.y - dot_red_from_other_camera.y) < 70))
			{
				dot_red_from_other_camera = dot_red_local;
				// memcpy(&dot_red_from_other_camera, &dot_red_local, sizeof(struct dot_t));
				Green_Dot_X_Estimated_Error = 0;
				Green_Dot_Y_Estimated_Error = 0;
				SET_EVENT(GOT_DOT_RED);
			}
			// else
			// {
			// 	DrawString(2, 0, "               ");
			// }
		}

		// get the position of motor
		static uint8_t data_send_to_motor[3] = {0, 0x36, 0x6B};
		data_send_to_motor[0] = red_x_stepper_motor_handle->address;
		red_x_stepper_motor_handle->Send_Instruction(red_x_stepper_motor_handle, data_send_to_motor, 3);
		data_send_to_motor[0] = red_y_stepper_motor_handle->address;
		red_y_stepper_motor_handle->Send_Instruction(red_y_stepper_motor_handle, data_send_to_motor, 3);

		pos_red_x_local = red_x_stepper_motor_handle->get_current_position_from_buff(red_x_stepper_motor_handle);
		pos_red_y_local = red_y_stepper_motor_handle->get_current_position_from_buff(red_y_stepper_motor_handle);

		if (pos_red_x_local != -0x3f3f3f3f)
		{
			pos_red_x_motor = (int)(pos_red_x_local * pulse_per_pixel);
			// sprintf(str,"red_x:%6d",pos_red_x_motor);
			// DrawString(4, 0, str);
		}
		if (pos_red_y_local != -0x3f3f3f3f)
		{
			pos_red_y_motor = (int)(pos_red_y_local * pulse_per_pixel);
			// sprintf(str,"red_y:%6d",pos_red_y_motor);
			// DrawString(5, 0, str);
		}

		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

/**
 * the following code is about the task that we create
 */

/**
 * @description: this task is is used to show necessary information on the OLED
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
static void OLED_SHOW(void *pvParameters)
{
	char buff[20];

	while (1)
	{
		sprintf(buff, "task");
		DrawString(0, 0, buff);
		UpdateScreenDisplay();
		vTaskDelay(200);
	}
}

/**
 * @description: this task is including the main logic of the program
 * @param {void} *parameter :this param is necessary for freertos task
 * @return {*}
 */
static void KEY_SCAN(void *parameter)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(10); // 10ms
	xLastWakeTime = xTaskGetTickCount();
	while (1)
	{
		/* 按键扫描 */
		bsp_KeyScan();
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

static void Task_schedule(void *pvParameters)
{
	const uint8_t detect_quadrangle_instruction[3] = {0X55, 0x01, 0X56};
	const uint8_t detect_red_dot_instruction[3] = {0X55, 0x02, 0X56};
	const uint8_t detect_two_dots_instruction[3] = {0X55, 0x03, 0X56};
	const uint8_t stop_detecting_instruction[3] = {0X55, 0x04, 0X56};
	uint8_t Key_Value = 0;
	while (1)
	{
		Key_Value = bsp_GetKey();
		// send the instruction to the upper computer
		if (CHECK_EVENT(GAME_OVER))
		{
			Usart_SendArray(UART5, stop_detecting_instruction, 3);
			switch (Key_Value)
			{
			case task1:
				DrawString(0, 9, "1");
				// 开始让激光回到原点
				CLEAR_EVENT(GAME_OVER);
				vTaskResume(Return_The_Original_Point_Handle);
				break;
			case task2:
				// 开始走外面的铅笔线
				DrawString(0, 9, "2");
				CLEAR_EVENT(GAME_OVER);
				vTaskResume(Follow_pencil_line_Handle);
				break;
			case task3:
				// 开始走里面的A4纸线
				DrawString(0, 9, "3");
				do
				{
					vPortEnterCritical();
					Usart_SendArray(UART5, detect_quadrangle_instruction, 3);
					vPortExitCritical();
					vTaskDelay(100);
				} while (!CHECK_EVENT(GOT_QUADRANGLE));
				CLEAR_EVENT(GAME_OVER);
				vTaskResume(Follow_By_Quadrangle_Handle);
				break;
			case task4:
				// 开始走不规则摆放里面的A4纸线
				DrawString(0, 9, "4");
				do
				{
					vPortEnterCritical();
					Usart_SendArray(UART5, detect_quadrangle_instruction, 3);
					vPortExitCritical();
					vTaskDelay(100);
				} while (!CHECK_EVENT(GOT_QUADRANGLE));
				CLEAR_EVENT(GAME_OVER);
				vTaskResume(Follow_By_Quadrangle_Handle);
				break;
			case task5:
				// 开始发挥题，回到原点
				DrawString(0, 9, "5");
				do
				{
					vPortEnterCritical();
					Usart_SendArray(UART5, detect_red_dot_instruction, 3);
					vPortExitCritical();
					vTaskDelay(100);
				} while (!CHECK_EVENT(GOT_DOT_RED));
				SET_EVENT(FOLLWING);
				CLEAR_EVENT(GAME_OVER);
				vTaskResume(Return_The_Original_Point_Handle);
				vTaskResume(Follow_Red_dot_Handle);
				// !the code is not complete, we need to resume another task to achieve that green dot is following the red dot
				break;
			case task6:
				// 开始发挥题，走里面的A4纸线
				DrawString(0, 9, "6");
				do
				{
					vPortEnterCritical();
					Usart_SendArray(UART5, detect_red_dot_instruction, 3);
					vPortExitCritical();
					vTaskDelay(100);
				} while (!CHECK_EVENT(GOT_DOT_RED));
				do
				{
					vPortEnterCritical();
					Usart_SendArray(UART5, detect_quadrangle_instruction, 3);
					vPortExitCritical();
					vTaskDelay(100);
				} while (!CHECK_EVENT(GOT_QUADRANGLE));
				CLEAR_EVENT(GAME_OVER);
				SET_EVENT(FOLLWING);
				vTaskResume(Follow_Red_dot_Handle);
				vTaskDelay(200);
				vTaskResume(Follow_By_Quadrangle_Handle);
				// !the code is not complete, we need to resume another task to achieve that green dot is following the red dot
				break;
			default:
				break;
			}
		}

		vTaskDelay(100);
	}
}

static void Follow_pencil_line(void *pvParameters)
{
	for (;;)
	{
		Stepper_Achieve_Distance_In_specific_Speed(red_x_stepper_motor_handle, Stepper_Backward, 2000, 5);
		Delayms(750);
		Stepper_Achieve_Distance_In_specific_Speed(red_y_stepper_motor_handle, Stepper_Backward, 1700, 5);
		Delayms(1500 / 2);
		Stepper_Achieve_Distance_In_specific_Speed(red_x_stepper_motor_handle, Stepper_Forward, 4050, 5);
		Delayms(3000 / 2);
		Stepper_Achieve_Distance_In_specific_Speed(red_y_stepper_motor_handle, Stepper_Forward, 3600, 5);
		Delayms(3000 / 2);
		Stepper_Achieve_Distance_In_specific_Speed(red_x_stepper_motor_handle, Stepper_Backward, 3950, 5);
		Delayms(3000 / 2);
		Stepper_Achieve_Distance_In_specific_Speed(red_y_stepper_motor_handle, Stepper_Backward, 3500, 5);

		SET_EVENT(GAME_OVER);
		vTaskSuspend(Follow_pencil_line_Handle);
	}
}

static void Follow_By_Quadrangle(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(30); // 40ms
	xLastWakeTime = xTaskGetTickCount();

	static int which_line = 0;
	static int which_dot_on_line = 0;
	static struct dot_t aim_dot;

	static int x_error = 0;
	static int y_error = 0;
	static int x_target = 0;
	static int y_target = 0;

	static int arrive_index = 0;

	for (;;)
	{
		// this section is used to stop the task, the judgement will never be true, the only way to stop the task is to use goto
		if (0)
		{
		stop_this_task:
			vTaskSuspend(Follow_By_Quadrangle_Handle);
			CLEAR_EVENT(FOLLWING);
			xLastWakeTime = xTaskGetTickCount();
		}
		vPortEnterCritical();
		aim_dot = (quadrangle_handle->GetSpectialDotOnLines(quadrangle_handle, which_line, which_dot_on_line));
		vPortExitCritical();
		// 1. get error
		x_error = ((dot_red_real.x + (int)Red_Dot_X_Estimated_Error) - aim_dot.x);
		y_error = ((dot_red_real.y + (int)Red_Dot_Y_Estimated_Error) - aim_dot.y);

		// 2. update the aim dot
		// if the dot have ran over the aim point, we need to update the aim point
		if (which_dot_on_line || which_line)
		{
			static struct dot_t last;
			if (which_dot_on_line)
			{
				last = (quadrangle_handle->GetSpectialDotOnLines(quadrangle_handle, which_line, which_dot_on_line - 1));
			}
			else // if (which_line)
			{
				last = (quadrangle_handle->GetSpectialDotOnLines(quadrangle_handle, which_line - 1, DOT_NUM - 1));
			}
			if ((last.x - aim_dot.x) * (dot_red_real.x - aim_dot.x) < 0 && (last.y - aim_dot.y) * (dot_red_real.y - aim_dot.y) < 0)
			{
				which_dot_on_line++;
				if (which_dot_on_line == DOT_NUM)
				{
					which_dot_on_line = 0;
					which_line++;
					if (which_line == 4)
					{
						which_line = 0;
						vTaskDelay(700);
						SET_EVENT(GAME_OVER);
						// the delay in here is used for waiting linux upper got the instruction that the task is over
						vTaskDelay(100);
						CLEAR_EVENT(GOT_QUADRANGLE);
						goto stop_this_task;
					}
				}
				aim_dot = (quadrangle_handle->GetSpectialDotOnLines(quadrangle_handle, which_line, which_dot_on_line));
				goto out;
			}
		}

		if ((abs(x_error) < 5) && (abs(y_error) < 5))
		{
			arrive_index++;
			if (arrive_index > 2)
			{
				arrive_index = 0;
				which_dot_on_line++;
				if (which_dot_on_line == DOT_NUM)
				{
					which_dot_on_line = 0;
					which_line++;
					if (which_line == 4)
					{
						which_line = 0;
						vTaskDelay(700);
						SET_EVENT(GAME_OVER);

						// the delay in here is used for waiting linux upper got the instruction that the task is over
						vTaskDelay(100);
						CLEAR_EVENT(GOT_QUADRANGLE);
						goto stop_this_task;
					}
				}
				// aim_dot = (quadrangle_handle->dots_on_lines[which_line][which_dot_on_line]);
				aim_dot = (quadrangle_handle->GetSpectialDotOnLines(quadrangle_handle, which_line, which_dot_on_line));
				goto out;

				// get the error again
				x_error = (dot_red_real.x - aim_dot.x);
				y_error = (dot_red_real.y - aim_dot.y);
			}
		}
		if (which_dot_on_line == DOT_NUM - 1 || (which_dot_on_line == 0 && which_line))
			if (abs(x_error) < 8 && abs(y_error) < 8)
			{
				which_dot_on_line = 1;
				which_line++;
				if (which_line == 4)
				{
					which_line = 0;
					which_dot_on_line = 0;
					vTaskDelay(700);
					SET_EVENT(GAME_OVER);
					// the delay in here is used for waiting linux upper got the instruction that the task is over
					vTaskDelay(100);
					CLEAR_EVENT(GOT_QUADRANGLE);
					goto stop_this_task;
				}
			}

		// 3. calculate the target position by PID
		x_target = PID_Realize(X_PID_handle, x_error);
		y_target = PID_Realize(Y_PID_handle, y_error);
		if (which_dot_on_line == 3)
			y_target = PID_Realize(X_PID_handle, y_error), x_target = PID_Realize(Y_PID_handle, x_error);
		// App_Printf("ex:%3d, ey:%3d\n", x_target, y_target);

		static int32_t max_target = 0;
		max_target = abs(x_target) > abs(y_target) ? abs(x_target) : abs(y_target);
		//		max_target = abs(max_target);
		if (max_target > 8)
		{
			x_target = x_target * 8 / max_target;
			y_target = y_target * 8 / max_target;
		}

		// 4. set the target position
		vPortEnterCritical();
		red_x_stepper_motor_handle->Achieve_Distance(red_x_stepper_motor_handle, (x_target > 0) ? (Stepper_Forward) : (Stepper_Backward), (x_target > 0) ? x_target : -x_target);
		red_y_stepper_motor_handle->Achieve_Distance(red_y_stepper_motor_handle, (y_target > 0) ? (Stepper_Forward) : (Stepper_Backward), (y_target > 0) ? y_target : -y_target);
		vPortExitCritical();

		// 5. estimate the red dot position
		Red_Dot_X_Estimated_Error += x_target * pulse_per_pixel;
		Red_Dot_Y_Estimated_Error += y_target * pulse_per_pixel;

	out:;
		CLEAR_EVENT(GOT_QUADRANGLE);
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

static void Return_The_Original_Point(void *pvParameters)
{
	const struct dot_t original_point = {152, 140}; // !the original point need to be set

	static int x_error = 0;
	static int y_error = 0;

	static int32_t x_PID_output_local = 0;
	static int32_t y_PID_output_local = 0;

	static int arrived_times = 0;

	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(30); // 10ms
	xLastWakeTime = xTaskGetTickCount();

	for (;;)
	{
		// this section is used to stop the task, the judgement will never be true, the only way to stop the task is to use goto
		if (0)
		{
		stop_this_task:
			vTaskDelay(700);
			SET_EVENT(GAME_OVER);

			// the delay in here is used for waiting linux upper got the instruction that the task is over
			vTaskDelay(100);

			CLEAR_EVENT(FOLLWING);
			vTaskSuspend(Return_The_Original_Point_Handle);
			xLastWakeTime = xTaskGetTickCount();
		}

		// 1. get error
		x_error = pos_red_x_motor;
		y_error = pos_red_y_motor;

		// 2. judge whether the target position is reached
		if ((abs(x_error) < 4) && (abs(y_error) < 4))
		{
			// stop the timer
			arrived_times++;
			if (arrived_times == 20)
			{
				arrived_times = 0;

				goto stop_this_task;
			}
		}

		// 3. calculate the target position by PID
		x_PID_output_local = PID_Realize(X_PID_handle, x_error);
		y_PID_output_local = PID_Realize(Y_PID_handle, y_error);

		// 4. set the target position
		vPortEnterCritical();
		red_x_stepper_motor_handle->Achieve_Distance(red_x_stepper_motor_handle, (x_PID_output_local > 0) ? (Stepper_Forward) : (Stepper_Backward), (x_PID_output_local > 0) ? x_PID_output_local : -x_PID_output_local);
		red_y_stepper_motor_handle->Achieve_Distance(red_y_stepper_motor_handle, (y_PID_output_local > 0) ? (Stepper_Forward) : (Stepper_Backward), (y_PID_output_local > 0) ? y_PID_output_local : -y_PID_output_local);
		vPortExitCritical();

	out:;
		// CLEAR_EVENT(GOT_DOT_RED);
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

static void Follow_Red_dot(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(10); // 10ms
	xLastWakeTime = xTaskGetTickCount();

	static int x_error = 0;
	static int y_error = 0;
	static int x_target = 0;
	static int y_target = 0;
	static int x_PID_output_local = 0;
	static int y_PID_output_local = 0;
	static int arrived_times = 0;
	static char str[33];

	for (;;)
	{
		// stop the task
		if (0)
		{
		stop_this_task:
			SET_EVENT(GAME_OVER);
			CLEAR_EVENT(GOT_DOT_RED);
			CLEAR_EVENT(GOT_DOT_GREEN);
			CLEAR_EVENT(GOT_QUADRANGLE);
			DrawString(3, 0, "            ");
			vTaskSuspend(Follow_Red_dot_Handle);
			xLastWakeTime = xTaskGetTickCount();
		}
		if (!CHECK_EVENT(FOLLWING))
			goto stop_this_task;
		// 1. get error
		x_error = (-dot_red_real.x + Green_Dot_X_Estimated_Error + dot_red_from_other_camera.x - 0);
		y_error = -(-dot_red_real.y + Green_Dot_Y_Estimated_Error + dot_red_from_other_camera.y - 0);

		// sprintf(str, "x:%d, y:%d", x_error, y_error);
		// DrawString(3, 0, str);

		// 2. calculate the target position by PID
		x_PID_output_local = PID_Realize(Green_X_PID_handle, x_error);
		y_PID_output_local = PID_Realize(Green_Y_PID_handle, y_error);

		static int32_t max_target = 0;
		max_target = abs(x_PID_output_local) > abs(y_PID_output_local) ? abs(x_PID_output_local) : abs(y_PID_output_local);
		if (max_target > 7)
		{
			x_PID_output_local = x_PID_output_local * 7 / max_target;
			y_PID_output_local = y_PID_output_local * 7 / max_target;
		}
		App_Printf("ex:%3d, ey:%3d\n", x_PID_output_local, y_PID_output_local);

		// 3. set the target position
		vPortEnterCritical();
		Stepper_stop_protect(green_x_stepper_motor_handle);
		green_x_stepper_motor_handle->Achieve_Distance(green_x_stepper_motor_handle, (x_PID_output_local > 0) ? (Stepper_Forward) : (Stepper_Backward), (x_PID_output_local > 0) ? x_PID_output_local : -x_PID_output_local);
		green_y_stepper_motor_handle->Achieve_Distance(green_y_stepper_motor_handle, (y_PID_output_local > 0) ? (Stepper_Forward) : (Stepper_Backward), (y_PID_output_local > 0) ? y_PID_output_local : -y_PID_output_local);
		vPortExitCritical();

		// 4. estimate the green dot position
		Green_Dot_X_Estimated_Error += x_PID_output_local * pulse_per_pixel_green;
		Green_Dot_Y_Estimated_Error -= y_PID_output_local * pulse_per_pixel_green;

	out:;
		CLEAR_EVENT(GOT_DOT_GREEN);
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}

/**
 * the code that create the task is end in here
 */

static void USER_Init(void)
{
#if configGENERATE_RUN_TIME_STATS && configUSE_STATS_FORMATTING_FUNCTIONS
	vSetupSysInfoTest();
#endif

	red_x_stepper_motor_handle = Stepper_Init(USART2, 0x01, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);
	red_y_stepper_motor_handle = Stepper_Init(UART4, 0x02, U4_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);
	green_x_stepper_motor_handle = Stepper_Init(USART2, 0x03, U2_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);
	green_y_stepper_motor_handle = Stepper_Init(UART4, 0x04, U4_buffer_handle, Stepper_Check_Way_0X6B, Stepper_FOC_Version_5_0);

	quadrangle_handle = Quadrangle_Init();

	// pid初始化
	PID_Initialize(X_PID_handle, .6 / 1.15 * 1.4, 0., 0., 0, 0, -0);
	PID_Initialize(Y_PID_handle, .53 / 1.2 * 1.4, 0., 0., 0, 0, -0);
	PID_Initialize(Green_X_PID_handle, .25, 0.14, 0., 0, 0, -0);
	PID_Initialize(Green_Y_PID_handle, .25, 0.14, 0., 0, 0, -0);
}

/***********************************************************************
 * @ 函数名  ： BSP_Init
 * @ 功能说明： 板级外设初始化，所有板子上的初始化均可放在这个函数里面
 * @ 参数    ：
 * @ 返回值  ： 无
 ***********************************************************************/
static void BSP_Init(void)
{
	/*
	 * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
	 * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
	 * 都统一用这个优先级分组，千万不要再分组，切忌。
	 */
	RCC_ClocksTypeDef get_rcc_clock;
	RCC_GetClocksFreq(&get_rcc_clock);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	delay_init(168);
	bsp_InitKey();
	Delayms(1000);
	OLED_Init();

	Init_USART1_All(); //*调试信息输出
	Init_USART2_All(); //*USART2 red_dot_stepper_motor
	Init_UART4_All();  //*USART4 green_dot_stepper_motor
	Init_USART3_All();
	Init_UART5_All(); //*USART5 upper_computer

	Buzzer_ONE();
	Delayms(1);

	GPIO_SetBits(GPIOE, GPIO_Pin_1);
}

///********************************END OF FILE****************************/
