
#ifndef CRC
#define CRC

#include "stm32f4xx.h"
uint16_t crc16(uint8_t *data, int length);
void appendCRC(uint8_t *data, int length);
int verifyCRC(uint8_t *data, int length);

#endif
