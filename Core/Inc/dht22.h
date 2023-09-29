#ifndef __DHT22_H
#define __DHT22_H
#include "stm32f1xx_hal.h"



#define DHT22_PORT GPIOB
#define DHT22_PIN GPIO_PIN_9

uint8_t DHT22_Start(void);
uint8_t DHT22_Read(void);	

#endif