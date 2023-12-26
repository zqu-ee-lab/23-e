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

#define pulse_per_pixel .12

QueueHandle_t Task_Number_Handle = NULL;

static TaskHandle_t analyse_data_Handle = NULL;  //+解析数据定时器句柄
static TaskHandle_t OLED_SHOW_Handle = NULL;     //+OLDE显示句柄
static TaskHandle_t AppTaskCreate_Handle = NULL; //+创建任务句柄
static TaskHandle_t KEY_SCAN_Handle = NULL;      //+KEY_SCAN句柄
static TaskHandle_t Task_schedule_Handle = NULL; //+SysInfoTestSent句柄
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
struct quadrangle_t *quadrangle_handle = NULL;

struct dot_t dot_red;
struct dot_t dot_green;

struct dot_t pencil_line[4] = {
    {.x = 19, .y = 271},
    {.x = 27, .y = 21},
    {.x = 285, .y = 271},
    {.x = 278, .y = 21},
};

struct dot_t *dot_red_handle = &dot_red;
struct dot_t *dot_green_handle = &dot_green;

struct PID X_PID;
struct PID Y_PID;

struct PID *X_PID_handle = &X_PID;
struct PID *Y_PID_handle = &Y_PID;

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
static void Follow_By_Quadrangle(void *pvParameters);
static void AppTaskCreate(void);               /* 用于创建任务 */
static void Task__TWO(void *pvParameters);     /* Test_Task任务实现 */
static void OLED_SHOW(void *pvParameters);     /* Test_Task任务实现 */
static void KEY_SCAN(void *pvParameters);      /* Test_Task任务实现 */
static void Task_schedule(void *pvParameters); /* 任务调度任务实现 */
static void Follow_Red_dot(void *pvParameters);
static void BSP_Init(void);                    /* 用于初始化板载相关资源 */
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
    xReturn = xTaskCreate((TaskFunction_t)AppTaskCreate,          /* 任务入口函数 */
                          (const char *)"AppTaskCreate",          /* 任务名字 */
                          (uint16_t)1024,                         /* 任务栈大小 */
                          (void *)NULL,                           /* 任务入口函数参数 */
                          (UBaseType_t)1,                         /* 任务的优先级 */
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
    xReturn = xTaskCreate((TaskFunction_t)SysInfoTestSent,          /* 任务入口函数 */
                          (const char *)"SysInfoTestSent",          /* 任务名字 */
                          (uint16_t)512,                            /* 任务栈大小 */
                          (void *)NULL,                             /* 任务入口函数参数 */
                          (UBaseType_t)1,                           /* 任务的优先级 */
                          (TaskHandle_t *)&SysInfoTestSent_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        printf("SysInfoTestSent任务创建成功\r\n");
#endif
    xReturn = xTaskCreate((TaskFunction_t)KEY_SCAN,          /* 任务入口函数 */
                          (const char *)"KEY_SCAN",          /* 任务名字 */
                          (uint16_t)512,                     /* 任务栈大小 */
                          (void *)NULL,                      /* 任务入口函数参数 */
                          (UBaseType_t)10,                   /* 任务的优先级 */
                          (TaskHandle_t *)&KEY_SCAN_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        App_Printf("KEY_SCAN任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)OLED_SHOW,          /* 任务入口函数 */
                          (const char *)"OLED_SHOW",          /* 任务名字 */
                          (uint16_t)512,                      /* 任务栈大小 */
                          (void *)NULL,                       /* 任务入口函数参数 */
                          (UBaseType_t)3,                     /* 任务的优先级 */
                          (TaskHandle_t *)&OLED_SHOW_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        App_Printf("OLED_SHOW任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)analyse_data,
                          (const char *)"analyse_data",
                          (uint16_t)256,                         /* 任务栈大小 */
                          (void *)NULL,                          /* 任务入口函数参数 */
                          (UBaseType_t)7,                        /* 任务的优先级 */
                          (TaskHandle_t *)&analyse_data_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        App_Printf("analyse_data任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Task_schedule,
                          (const char *)"Task_schedule",
                          (uint16_t)256,                          /* 任务栈大小 */
                          (void *)NULL,                           /* 任务入口函数参数 */
                          (UBaseType_t)5,                         /* 任务的优先级 */
                          (TaskHandle_t *)&Task_schedule_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        App_Printf("Task_schedule任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Follow_By_Quadrangle,
                          (const char *)"Follow_By_Quadrangle",
                          (uint16_t)256,                                 /* 任务栈大小 */
                          (void *)NULL,                                  /* 任务入口函数参数 */
                          (UBaseType_t)4,                                /* 任务的优先级 */
                          (TaskHandle_t *)&Follow_By_Quadrangle_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        App_Printf("Follow_By_Quadrangle任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Return_The_Original_Point,
                          (const char *)"Return_The_Original_Point",
                          (uint16_t)256,                                      /* 任务栈大小 */
                          (void *)NULL,                                       /* 任务入口函数参数 */
                          (UBaseType_t)4,                                     /* 任务的优先级 */
                          (TaskHandle_t *)&Return_The_Original_Point_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        App_Printf("Return_The_Original_Point任务创建成功\r\n");
    xReturn = xTaskCreate((TaskFunction_t)Follow_Red_dot,
                          (const char *)"Follow_Red_dot",
                          (uint16_t)256,                              /* 任务栈大小 */
                          (void *)NULL,                               /* 任务入口函数参数 */
                          (UBaseType_t)4,                             /* 任务的优先级 */
                          (TaskHandle_t *)&Follow_Red_dot_Handle); /* 任务控制块指针 */
    if (xReturn == pdPASS)
        App_Printf("Follow_Red_dot任务创建成功\r\n");

    Task_Number_Handle = xQueueCreate(1, 1); // 开始解析数据
    Group_One_Handle = xEventGroupCreate();
    SET_EVENT(GAME_OVER);

    // 挂机任务，等待选择任务
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
    const TickType_t xFrequency = pdMS_TO_TICKS(10); // 10ms
    xLastWakeTime = xTaskGetTickCount();
    const char head_red_laser[2] = {0xFE, 0xFE};
    const char head_green_laser[2] = {0xFE, 0xFD};
    const char head_quadrangle[2] = {0xFE, 0xFF};

    struct quadrangle_t *quadrangle_local_handle = Quadrangle_Init();
    struct dot_t dot_red_local;
    struct dot_t dot_green_local;

    for (;;)
    {
        /* code */

        // usart2 get data
        // get the quadrangle data
        if (BUFF_pop_with_check_by_Protocol(U5_buffer_handle, head_quadrangle, 2, quadrangle_local_handle->dots, 16, 1, 8) == 8)
        {
            App_Printf("get quadrangle data\r\n");
            quadrangle_local_handle->Sort(quadrangle_local_handle);
            quadrangle_local_handle->Equal_Scaling(quadrangle_local_handle, .95);
            quadrangle_local_handle->GetDotsOnLines(quadrangle_local_handle);
            memcpy(quadrangle_handle, quadrangle_local_handle, sizeof(struct quadrangle_t));
            App_Printf("quadrangle data is:\n%d %d \n%d %d\n%d %d \n%d %d\n", quadrangle_handle->dots[0].x, quadrangle_handle->dots[0].y,
                       quadrangle_handle->dots[1].x, quadrangle_handle->dots[1].y, quadrangle_handle->dots[2].x, quadrangle_handle->dots[2].y,
                       quadrangle_handle->dots[3].x, quadrangle_handle->dots[3].y);
            SET_EVENT(GOT_QUADRANGLE);
        }

        // get the red laser dot
        if (BUFF_pop_with_check_by_Protocol(U5_buffer_handle, head_red_laser, 2, &dot_red_local, 16, 1, 2) == 2)
        {
            // App_Printf("get laser data\r\n");
            if ((dot_red_local.x) + (dot_red_local.y))
            {
                memcpy(&dot_red, &dot_red_local, sizeof(struct dot_t));
                Red_Dot_X_Estimated_Error = 0;
                Red_Dot_Y_Estimated_Error = 0;
                SET_EVENT(GOT_DOT_RED);
            }
        }

        // get the green laser dot
        if (BUFF_pop_with_check_by_Protocol(U5_buffer_handle, head_green_laser, 2, &dot_green_local, 16, 1, 2) == 2)
        {
            App_Printf("get green laser data\r\n");
            memcpy(&dot_green, &dot_green_local, sizeof(struct dot_t));
            Green_Dot_X_Estimated_Error = 0;
            Green_Dot_Y_Estimated_Error = 0;
            SET_EVENT(GOT_DOT_GREEN);
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
        sprintf(buff, "task:");
        DrawString(0, 0, buff);
        UpdateScreenDisplay();
        vTaskDelay(100);
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
                // 开始让激光回到原点
                App_Printf("task1\r\n");
                do
                {
                    Usart_SendArray(UART5, detect_red_dot_instruction, 3);
                    vTaskDelay(100);
                } while (!CHECK_EVENT(GOT_DOT_RED));
                CLEAR_EVENT(GAME_OVER);
                vTaskResume(Return_The_Original_Point_Handle);
                break;
            case task2:
                // 开始走外面的铅笔线
                App_Printf("task2\r\n");
                quadrangle_handle->unInit(quadrangle_handle);
                quadrangle_handle = Quadrangle_Init_With_Dots(pencil_line);
                do
                {
                    Usart_SendArray(UART5, detect_red_dot_instruction, 3);
                } while (!CHECK_EVENT(GOT_DOT_RED));
                CLEAR_EVENT(GAME_OVER);
                vTaskResume(Follow_By_Quadrangle_Handle);
                break;
            case task3:
                // 开始走里面的A4纸线
                App_Printf("task3\r\n");
                do
                {
                    Usart_SendArray(UART5, detect_quadrangle_instruction, 3);
                    vTaskDelay(100);
                } while (!CHECK_EVENT(GOT_QUADRANGLE));
                do
                {
                    Usart_SendArray(UART5, detect_red_dot_instruction, 3);
                    vTaskDelay(100);
                } while (!CHECK_EVENT(GOT_DOT_RED));
                CLEAR_EVENT(GAME_OVER);
                vTaskResume(Follow_By_Quadrangle_Handle);
                break;
            case task4:
                // 开始走不规则摆放里面的A4纸线
                App_Printf("task4\r\n");
                do
                {
                    Usart_SendArray(UART5, detect_quadrangle_instruction, 3);
                    vTaskDelay(100);
                } while (!CHECK_EVENT(GOT_QUADRANGLE));
                do
                {
                    Usart_SendArray(UART5, detect_red_dot_instruction, 3);
                    vTaskDelay(100);
                } while (!CHECK_EVENT(GOT_DOT_RED));
                CLEAR_EVENT(GAME_OVER);
                vTaskResume(Follow_By_Quadrangle_Handle);
                break;
            case task5:
                // 开始发挥题，回到原点
                App_Printf("task5\r\n");
                do
                {
                    Usart_SendArray(UART5, detect_quadrangle_instruction, 3);
                    vTaskDelay(100);
                } while (!CHECK_EVENT(GOT_QUADRANGLE));
                do
                {
                    Usart_SendArray(UART5, detect_two_dots_instruction, 3);
                    vTaskDelay(100);
                } while ((!CHECK_EVENT(GOT_DOT_RED)) && (!CHECK_EVENT(GOT_DOT_GREEN)));
                CLEAR_EVENT(GAME_OVER);
                vTaskResume(Return_The_Original_Point_Handle);
                // !the code is not complete, we need to resume another task to achieve that green dot is following the red dot
                break;
            case task6:
                // 开始发挥题，走里面的A4纸线
                App_Printf("task6\r\n");
                do
                {
                    Usart_SendArray(UART5, detect_quadrangle_instruction, 3);
                    vTaskDelay(100);
                } while (CHECK_EVENT(GOT_QUADRANGLE));
                do
                {
                    Usart_SendArray(UART5, detect_two_dots_instruction, 3);
                    vTaskDelay(100);
                } while ((!CHECK_EVENT(GOT_DOT_RED)) && (!CHECK_EVENT(GOT_DOT_GREEN)));
                CLEAR_EVENT(GAME_OVER);
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

static void Follow_By_Quadrangle(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(40); // 10ms
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
            xLastWakeTime = xTaskGetTickCount();
        }

        aim_dot = (quadrangle_handle->dots_on_lines[which_line][which_dot_on_line]);
        // 1. get error
        if (dot_red.x + dot_red.y == 0)
        {
            goto out;
        }
        x_error = ((dot_red.x + Red_Dot_X_Estimated_Error) - aim_dot.x);
        y_error = ((dot_red.y + Red_Dot_Y_Estimated_Error) - aim_dot.y);

        // 2. update the aim dot
        if (which_dot_on_line)
        {
            u16 end_point_error_spar = (dot_red.y - quadrangle_handle->dots_on_lines[which_line][DOT_NUM - 1].y) * (dot_red.y - quadrangle_handle->dots_on_lines[which_line][DOT_NUM - 1].y) +
                                       (dot_red.x - quadrangle_handle->dots_on_lines[which_line][DOT_NUM - 1].x) * (dot_red.x - quadrangle_handle->dots_on_lines[which_line][DOT_NUM - 1].x);
            if (end_point_error_spar < (y_error * y_error + x_error * x_error))
            {
                which_dot_on_line = DOT_NUM - 1;
                aim_dot = (quadrangle_handle->dots_on_lines[which_line][DOT_NUM - 1]);
            }
        }
        if ((abs(x_error) < 3) && (abs(y_error) < 3))
        {
            arrive_index++;
            if (arrive_index > 5)
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
                        SET_EVENT(GAME_OVER);

                        // the delay in here is used for waiting linux upper got the instruction that the task is over
                        vTaskDelay(100);
                        CLEAR_EVENT(GOT_DOT_RED);
                        CLEAR_EVENT(GOT_DOT_GREEN);
                        CLEAR_EVENT(GOT_QUADRANGLE);
                        goto stop_this_task;
                    }
                }
                aim_dot = (quadrangle_handle->dots_on_lines[which_line][which_dot_on_line]);

                // get the error again
                x_error = (dot_red.x - aim_dot.x);
                y_error = (dot_red.y - aim_dot.y);
            }
        }

        // 3. calculate the target position by PID
        x_target = PID_Realize(X_PID_handle, x_error);
        y_target = PID_Realize(Y_PID_handle, y_error);
        App_Printf("ey:%3d, ex:%3d\n", y_error, x_error);

        // 4. set the target position
        red_x_stepper_motor_handle->Achieve_Distance(red_x_stepper_motor_handle, (x_target > 0) ? (Stepper_Forward) : (Stepper_Backward), (x_target > 0) ? x_target : -x_target);
        red_y_stepper_motor_handle->Achieve_Distance(red_y_stepper_motor_handle, (y_target > 0) ? (Stepper_Forward) : (Stepper_Backward), (y_target > 0) ? y_target : -y_target);

        // 5. estimate the red dot position
        Red_Dot_X_Estimated_Error += x_target * pulse_per_pixel;
        Red_Dot_Y_Estimated_Error += y_target * pulse_per_pixel;

    out:;
        CLEAR_EVENT(GOT_DOT_RED);
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
    const TickType_t xFrequency = pdMS_TO_TICKS(25); // 10ms
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        // this section is used to stop the task, the judgement will never be true, the only way to stop the task is to use goto
        if (0)
        {
        stop_this_task:

            SET_EVENT(GAME_OVER);

            // the delay in here is used for waiting linux upper got the instruction that the task is over
            vTaskDelay(100);
            CLEAR_EVENT(GOT_DOT_RED);
            CLEAR_EVENT(GOT_DOT_GREEN);
            CLEAR_EVENT(GOT_QUADRANGLE);
            vTaskSuspend(Return_The_Original_Point_Handle);
            xLastWakeTime = xTaskGetTickCount();
        }

        // 1. get error
        x_error = ((dot_red.x + Red_Dot_X_Estimated_Error) - original_point.x);
        y_error = ((dot_red.y + Red_Dot_Y_Estimated_Error) - original_point.y);

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
        red_x_stepper_motor_handle->Achieve_Distance(red_x_stepper_motor_handle, (x_PID_output_local > 0) ? (Stepper_Forward) : (Stepper_Backward), (x_PID_output_local > 0) ? x_PID_output_local : -x_PID_output_local);
        red_y_stepper_motor_handle->Achieve_Distance(red_y_stepper_motor_handle, (y_PID_output_local > 0) ? (Stepper_Forward) : (Stepper_Backward), (y_PID_output_local > 0) ? y_PID_output_local : -y_PID_output_local);

        // 5. estimate the red dot position
        Red_Dot_X_Estimated_Error += x_PID_output_local * pulse_per_pixel;
        Red_Dot_Y_Estimated_Error += y_PID_output_local * pulse_per_pixel;

    out:;
        CLEAR_EVENT(GOT_DOT_RED);
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

    for (;;)
    {
        // stop the task
        if (0)
        {
        stop_this_task:
            //vTaskSuspend(Follow_By_Quadrangle_Handle);
            xLastWakeTime = xTaskGetTickCount();
        }

        // 1. get error

        // 2. calculate the target position by PID

        // 3. set the target position

        // 4. estimate the green dot position

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

    red_x_stepper_motor_handle = Stepper_Init(USART2, 0x01, U2_buffer_handle);
    red_y_stepper_motor_handle = Stepper_Init(UART4, 0x02, U4_buffer_handle);

    //red_x_stepper_motor_handle->Achieve_Distance(red_y_stepper_motor_handle, Stepper_Forward, 2500);
    // while(1)
    // {
    //     red_y_stepper_motor_handle->Achieve_Distance(red_y_stepper_motor_handle, Stepper_Forward, 10);
    //     Delayms(10);
    // }

    // while(1)
    // {
    //     Delayms(1000);
    // }
    quadrangle_handle = Quadrangle_Init();

    // pid初始化
    PID_Initialize(X_PID_handle, .8, 0., 0., 0, 0, -0);
    PID_Initialize(Y_PID_handle, .8, 0., 0., 0, 0, -0);
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
