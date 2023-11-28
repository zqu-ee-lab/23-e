#ifndef STEPPER_H
#define STEPPER_H

#include "stdbool.h"
#include "stm32f4xx.h"                  // Device header

enum Stepper_Direction_t
{
  Stepper_Forward = 0x00,
  Stepper_Backward = 0x01
};

enum Stepper_Check_Way_t
{
  Stepper_Check_Way_XOR = 0x00,
  Stepper_Check_Way_0X68 = 0x01
};

struct Steeper_t * Stepper_Init(USART_TypeDef *pUSARTx, uint8_t address);

struct Steeper_t
{
  USART_TypeDef *pUSARTx;
  uint8_t address;
  uint8_t acceleration;
  uint8_t period;
  enum Stepper_Check_Way_t check_way;

  struct Steeper_t * (*Init)(USART_TypeDef *pUSARTx, uint8_t address);
  void (*unInit)(struct Steeper_t *this);
  void (*Send_Instruction)(struct Steeper_t *this, uint8_t *data, uint32_t dataLen);
  void (*Achieve_Distance)(struct Steeper_t* this, enum Stepper_Direction_t direction, uint32_t distance);
};

void Stepper_Achieve_Distance(struct Steeper_t* this, enum Stepper_Direction_t direction, uint32_t distance);

#endif /* STEPPER_H */