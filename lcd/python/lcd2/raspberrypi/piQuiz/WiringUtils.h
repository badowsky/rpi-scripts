#ifndef WIRINGUTILS_H_
#define WIRINGUTILS_H_

/*
When using interrupts the only way to properly export gpio pins and start interrupts is via shell commands using the gpio program.

To inizialize a pin you have to use : 
gpio export [pin] [out/in]

To start an interrupt on a pin with a rising/falling/both edge you have to use : 
gpio edge [pin][falling/rising/both/none]

Note that the pin numbering is NOT the wiring pi numbering but the broadcom one.
Go to https://projects.drogon.net/raspberry-pi/wiringpi/functions/ for a well detailed explanation.
This functions just wrap around the system() stdlib function to offer a more wiringPi like interface when you work with interrupts.
*/

typedef enum{EDGE_FALLING,EDGE_RISING,EDGE_BOTH,EDGE_NONE} Edge;
typedef enum{MODE_OUT,MODE_IN} PinMode;

int setInterrupt(int bcmPin, Edge edge);
int isAValidBCMPin(int bcmPin);
int setPinMode(int bcmPin, PinMode mode);

#endif