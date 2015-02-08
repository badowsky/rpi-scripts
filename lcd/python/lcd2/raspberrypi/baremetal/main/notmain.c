

#include "gpio/Gpio.h"
#include "main/Common.h"
#include "time/Time.h"

#define PIN_NUMBER1 17
#define PIN_NUMBER2 22
#define STATUS_LED 16
#define INPUT_PIN 7

void notmain ( void )
{
    /*
    setMode(PIN_NUMBER1,PIN_OUT);
    setMode(PIN_NUMBER2,PIN_OUT);
    setMode(INPUT_PIN,PIN_IN);

    setMode(STATUS_LED,PIN_OUT);
    writeOnPin(STATUS_LED,PIN_LOW);
    */

    while(TRUE)
    {
        flashPin(STATUS_LED, 10, 1);
    }

    /*
    
    int count = 10;

   while(count > 0)
    {
        int i;
    	writeOnPin(PIN_NUMBER2,PIN_HIGH);
    	writeOnPin(PIN_NUMBER1,PIN_HIGH);
    	for (i = 0; i < 9999999; ++i)
    	{
    		nop();
    	}

    	writeOnPin(PIN_NUMBER2,PIN_LOW);
    	writeOnPin(PIN_NUMBER1,PIN_LOW);
    	for (i = 0; i < 9999999; ++i)
    	{
    		nop();
    	}
        count = count - 1;
    }

    while(TRUE)
    {
        if(readPinValue(INPUT_PIN) == PIN_LOW)
            writeOnPin(PIN_NUMBER1,PIN_HIGH);

        writeOnPin(PIN_NUMBER1,PIN_LOW);
    }
    */

}