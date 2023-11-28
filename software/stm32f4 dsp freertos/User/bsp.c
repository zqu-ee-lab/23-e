#include "sys.h"
#include "stdio.h"
#include "OLED.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "JY62.h"
#include "PID.h"
#include "Driver.h"
#include "Software_iic.h"
#include <math.h>
#include <stdbool.h>


void Read_RGB()
{
    IIC_Read_Len(0X5A, 0X0C, 3, (u8*)&RGB);
}
void Forward()
{
    Advance(2);
    Advance(3);
    Advance(4);
    Advance(5);
}
void Reverse()
{
    Back(2);
    Back(3);
    Back(4);
    Back(5);
}
void Turn_Right_Founction()
{
    Back(2);
    Advance(3);
    Advance(4);
    Back(5);
}
void Turn_Left_Founction()
{
    Advance(2);
    Back(3);
    Back(4);
    Advance(5);
}
void Walking_Right()
{
    Advance(2);
    Back(3);
    Advance(4);
    Back(5);
}
void Walking_Left()
{
    Back(2);
    Advance(3);
    Back(4);
    Advance(5);
}


