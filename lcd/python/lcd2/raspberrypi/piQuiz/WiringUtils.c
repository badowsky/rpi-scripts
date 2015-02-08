#include "WiringUtils.h"
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

int setInterrupt(int bcmPin, Edge edge)
{
	if(!isAValidBCMPin(bcmPin))
	{
		printf("Pin %d is not a valid gpio bcm pin\n",bcmPin);
		return -1;
	}

	char string[20];

	switch(edge)
	{
		case EDGE_FALLING : sprintf(string,"gpio edge %d falling",bcmPin); break;

		case EDGE_BOTH : sprintf(string,"gpio edge %d both",bcmPin); break;

		case EDGE_RISING : sprintf(string,"gpio edge %d rising",bcmPin); break;

		case EDGE_NONE : sprintf(string,"gpio edge %d none",bcmPin); break;

		default :
		{
			perror("No valid edge found\n");
			return -1;
			break;
		}
	}

	if(system(string) == -1)
	{
		printf("Pin %d could not be set on %d edge \n",bcmPin,edge);
		return -1;
	}

	
	return 1;
}

int isAValidBCMPin(int bcmPin)
{
	if(bcmPin == 4 || bcmPin == 17 || bcmPin == 18 || bcmPin == 21 || bcmPin == 27 || bcmPin == 22 || bcmPin == 23 || bcmPin == 24 || bcmPin == 25)
		return 1;

	return -1;
}

int setPinMode(int bcmPin, PinMode mode)
{
	if(!isAValidBCMPin(bcmPin))
	{
		printf("Pin %d is not a valid gpio bcm pin\n",bcmPin);
		return -1;
	}

	char string[20];

	if(mode == MODE_OUT)
	{
		sprintf(string,"gpio export %d out",bcmPin);
	}
	else if(mode == MODE_IN)
	{
		sprintf(string,"gpio export %d in",bcmPin);
	}
	else 
	{
		perror("Mode not found\n");
		return -1;
	}


	if(system(string) == -1)
	{
		printf("Pin %d could not be exported\n",bcmPin);
		return -1;
	}

    return 0;
}