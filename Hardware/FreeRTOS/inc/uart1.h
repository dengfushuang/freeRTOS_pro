
#ifndef __UART1_H 
#define __UART1_H 
#include "stdint.h"

#define UART1_FIFO_LENGTH         8

uint8_t UART1_Init(uint32_t BPS);
void UART1Putch(uint8_t Data);   
void UART1Put_str(uint8_t *Data, uint16_t NByte);
void UART1Write_Str(uint8_t *Data);

uint8_t UART1Getch(void);
uint8_t UART1Get(void);

#endif






