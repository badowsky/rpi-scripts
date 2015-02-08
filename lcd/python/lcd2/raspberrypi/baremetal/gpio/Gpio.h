#ifndef GPIO_H_
#define GPIO_H_

#include "main/Common.h"

#define GPIO_BASE 0x20200000
#define PIN_LOW 0x0
#define PIN_HIGH 0x1

typedef enum pinFunction
{
	PIN_IN = 0x0,
	PIN_OUT = 0x1,
	PIN_A_0 = 0x4,
	PIN_A_1 = 0x5,
	PIN_A_2 = 0x6,
	PIN_A_3 = 0x7,
	PIN_A_4 = 0x3, 
	PIN_A_5 = 0x2
}pinFunction;

bool setMode(int pinNumber,int function);
bool writeOnPin(int pinNumber, int val);
int readPinValue(int pinNumber);
bool flashPin(int pinNumber, int delay, int times);

#endif