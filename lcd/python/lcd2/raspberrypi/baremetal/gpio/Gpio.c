#include "gpio/Gpio.h"
#include "main/Common.h"
#include "memory/MemoryOp.h"

extern void delaySeconds(unsigned int time);

bool setMode(int pinNumber,int function)
{
	if(function < 0 && function > 7)
		return FALSE;

	if(pinNumber < 1 && pinNumber > 54)
		return FALSE;
	
	unsigned int fselAddress = GPIO_BASE;

	while(pinNumber > 9)
	{
		pinNumber = pinNumber - 10;
		fselAddress += 0x4;
	}
	
	pinNumber = pinNumber * 3;

	unsigned int fselVal = read(fselAddress);

    unsigned int newVal = fselVal & (~(0x7<<pinNumber));
    newVal = fselVal | function<<pinNumber;

    store(fselAddress,newVal);

    return TRUE;
}

bool writeOnPin(int pinNumber, int val)
{
	if(pinNumber < 1 && pinNumber > 54)
		return FALSE;
    
    unsigned int address = 0x20200028;

    if(val > 0)
    {
    	address = 0x2020001C;
    }

    if(pinNumber > 32)
    {
    	pinNumber = pinNumber & 32;
    	address = address + 0x4;
    }

	store(address,0x1<<pinNumber);

	return TRUE;
}

int readPinValue(int pinNumber)
{
	if(pinNumber < 1 && pinNumber > 54)
		return FALSE;

	unsigned int address = 0x20200034;

	if(pinNumber > 32)
    {
    	pinNumber = pinNumber - 32;
    	address = address + 0x4;
    }

    int pinVal = read(address);

    if(pinVal & (0x1<<pinNumber))
        return TRUE;

    return FALSE;
}

bool flashPin(int pinNumber, int delay, int times)
{
	if(pinNumber < 1 && pinNumber > 54)
		return FALSE;

	if(times <= 0)
		return FALSE;

	setMode(pinNumber, PIN_OUT);

	int counter = 0;

	while(counter < times)
    {
        writeOnPin(pinNumber,PIN_LOW);
        delaySeconds(delay);

        writeOnPin(pinNumber,PIN_HIGH);
        delaySeconds(delay);

        counter = counter + 1;
    }

    return TRUE;
}






