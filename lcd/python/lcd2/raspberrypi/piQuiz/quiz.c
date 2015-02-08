#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include "WiringUtils.h"

#define NUMBER_OF_QUESTIONS 4
#define PIN_USED 4
#define BUZZER_PIN 18
#define BUTTON_PIN 17


typedef enum{false,true} bool;

void initWiringPi();

int main()
{
	initWiringPi();

	int leds[PIN_USED];

	leds[0] = 23;
	leds[1] = 24;
	leds[2] = 22;
	leds[3] = 25;

    int l;

    //set pins to out
	for(l = 0; l < PIN_USED; l++)
    {
    	if(setPinMode(leds[l],MODE_OUT) == -1)
        {
    	  printf("Pin %d not exported\n",leds[l]);
    	  exit(1);
        }
    }

    char * questions[NUMBER_OF_QUESTIONS];
    bool questionVal[NUMBER_OF_QUESTIONS];

    questions[0] = "The idea for the raspberry Pi came to Eben in 2004";
    questionVal[0] = false;

    questions[1] = "Raspbian is the default distro";
    questionVal[1] = true;

    questions[2] = "The first alpha boards were created in 2011";
    questionVal[2] = true;

    questions[3] = "The raspberry pi is built exclusively in China";
    questionVal[3] = false;

    //the current question to ask
	int questioN = 0;

	bool continueGame = false;

	int i = 0;

	while(questioN < NUMBER_OF_QUESTIONS)
    {
       printf("%s\n",questions[questioN]);

       continueGame = false;

       int res = waitForInterrupt(BUTTON_PIN,2500);

	   if(res == 1 && questionVal[questioN] == true)
	   {
	   	  printf("Right answer\n");
	   	  digitalWrite(leds[i],1);
	   	  continueGame = true;
	   }
	
	   if(res == 0 && questionVal[questioN] == false)
	   {
	   	  printf("Right answer\n");
	   	  digitalWrite(leds[i],1);
		  continueGame = true;
	   }

       if(continueGame == false)
       {
       	  printf("Wrong answer\n");
	      digitalWrite(BUZZER_PIN,1);
	      delay(1000);
	      digitalWrite(BUZZER_PIN,0);
	      break;
	   }

	   ++questioN;
	   ++i;

	   delay(1000);
    }

    if(continueGame)
    	printf("You won !!\n");
	else
		printf("You have lost the game :(\n");
    
    //shut down all the leds
    for(i = 0; i < 4; i++)
    {
        digitalWrite(leds[i],0);
    }

	return 0;
}

void initWiringPi()
{
	if(wiringPiSetupSys() == -1)
	{
		perror("Error while initializing wiringPi\n");
		exit(1);
	}

    if(setPinMode(BUTTON_PIN,MODE_IN) == -1)
    {
    	perror("Button pin not exported\n");
    	exit(1);
    }

    if(setPinMode(BUZZER_PIN,MODE_OUT) == -1)
    {
    	perror("Buzzer pin not exported\n");
    	exit(1);
    }

	if(setInterrupt(BUTTON_PIN,EDGE_FALLING) == -1)
	{
		perror("Interrupt not set on button pin\n");
		exit(1);
	}

}
