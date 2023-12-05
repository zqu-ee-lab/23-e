#include "Stepper.h" // User/stepper/Stepper.h
#include "usart.h"   // User/usart.h
#include "FreeRTOS.h"
#include "delay.h"
#include "buffer.h"

void Stepper_Send_Instruction(struct Steeper_t *this, uint8_t *data, uint32_t dataLen)
{
    Usart_SendArray(this->pUSARTx, data, dataLen);
}

void Stepper_Achieve_Distance(struct Steeper_t *this, enum Stepper_Direction_t direction, uint32_t distance)
{
    int16_t speed = 150;
    // calculate speed according to the period

    uint8_t data[13] = {0};
    data[0] = this->address;
    data[1] = 0xFD; // Distance controlled instruction
    data[2] = direction;

    data[3] = speed >> 8;
    data[4] = speed;

    data[5] = this->acceleration;

    data[6] = distance >> 24;
    data[7] = distance >> 16;
    data[8] = distance >> 8;
    data[9] = distance;

    data[10] = 0x00;
    data[11] = 0x00; // multiple instructions

    if (this->check_way == Stepper_Check_Way_XOR)
    {
        uint8_t check = 0;
        for (int i = 0; i < 12; i++)
        {
            check ^= data[i];
        }
        data[12] = check;
    }
    else if (this->check_way == Stepper_Check_Way_0X6B)
    {
        data[12] = 0x6B;
    }
    else
    {
        // error
        App_Printf("Stepper_Achieve_Distance: check way error!\n");
        return;
    }

    this->Send_Instruction(this, data, 13);
    return;
}

int32_t Stepper_Read_Current_Position(struct Steeper_t *this)
{
    uint8_t data[3] = {0};
    data[0] = this->address;
    data[1] = 0x36; // Read current position instruction

    if (this->check_way == Stepper_Check_Way_XOR)
    {
        uint8_t check = 0;
        for (int i = 0; i < 2; i++)
        {
            check ^= data[i];
        }
        data[2] = check;
    }
    else if (this->check_way == Stepper_Check_Way_0X6B)
    {
        data[2] = 0x6B;
    }
    else
    {
        // error
        App_Printf("Stepper_Read_Current_Position: check way error!\n");
        return 0;
    }

    this->Send_Instruction(this, data, 6);
    Delayms(10); // wait for the response

    int32_t current_position = 0;
    uint8_t data_res[5] = {0};
    char head[2] = {this->address, 0x36};
    if(BUFF_pop_by_Protocol(this->BUFF, head, 2, data_res, 5)==5)
    {
        current_position = (data_res[1] << 24) | (data_res[2] << 16) | (data_res[3] << 8) | data_res[4];
        current_position = current_position / 65535 * 200 * 256;
        current_position = data_res[0]?-current_position:current_position;
        return current_position;
    }
    else
    {
        App_Printf("Stepper_Read_Current_Position: pop error!\n");
        return 0;
    }


    return 0;
}

void Stepper_unInit(struct Steeper_t *this)
{
    vPortFree(this);
}

struct Steeper_t *Stepper_Init(USART_TypeDef *pUSARTx, uint8_t address, struct Buff *BUFF)
{
    struct Steeper_t *this = NULL;
    this = (struct Steeper_t *)pvPortMalloc(sizeof(struct Steeper_t));
    if (this == NULL)
    {
        while (1)
        {
            ;
        }
    }
    this->pUSARTx = pUSARTx;
    this->address = address;
    this->BUFF = BUFF;
    this->acceleration = 0x00;               // default acceleration
    this->period = 10;                       // unit : ms
    this->check_way = Stepper_Check_Way_0X6B; // default check way

    this->Init = Stepper_Init;
    this->unInit = Stepper_unInit;
    this->Read_Current_Position = Stepper_Read_Current_Position;
    this->Achieve_Distance = Stepper_Achieve_Distance;
    this->Send_Instruction = Stepper_Send_Instruction;

    return this;
}
