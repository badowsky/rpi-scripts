#include "Lcd.h"
#include "wiringPi.h"

void setDefaultHd44780(hd44780 * toDefault)
{
	if(!toDefault)
		return;

	toDefault->D4 = 26;
	toDefault->D5 = 19;
	toDefault->D6 = 13;
	toDefault->D7 = 6;

	toDefault->registerSelect = 21;
	toDefault->enable = 16;

	toDefault->colNumber = 2;
	toDefault->rowNumber = 16;

}

void initializeDisplay(hd44780 * header)
{
	if(!header)
		return;

	wiringPiSetupGpio();

	pinMode(header->D4,OUTPUT);
	pinMode(header->D5,OUTPUT);
	pinMode(header->D6,OUTPUT);
	pinMode(header->D7,OUTPUT);

	pinMode(header->registerSelect,OUTPUT);
	pinMode(header->enable,OUTPUT);
	
}

static void pulse(hd44780 * header)
{
	delay(1);

	digitalWrite(header->enable,LOW);

    delay(1);

	digitalWrite(header->enable,HIGH);

	delay(1);

	digitalWrite(header->enable,LOW);

	delay(1);

}

void writeBytes(hd44780 * header, int byte, int mode)
{
	if(!header)
		return;

    digitalWrite(header->registerSelect, mode);

	delay(1);

	digitalWrite(header->D4,LOW);
	digitalWrite(header->D5,LOW);
	digitalWrite(header->D6,LOW);
	digitalWrite(header->D7,LOW);


	if(byte & 0x10)
		digitalWrite(header->D4,HIGH);

	if(byte & 0x20)
		digitalWrite(header->D5,HIGH);

	if(byte & 0x40)
		digitalWrite(header->D6,HIGH);
	if(byte & 0x80)
		digitalWrite(header->D7,HIGH);
	

	pulse(header);


	digitalWrite(header->D4,LOW);
	digitalWrite(header->D5,LOW);
	digitalWrite(header->D6,LOW);
	digitalWrite(header->D7,LOW);

	if(byte & 0x01)
		digitalWrite(header->D4,HIGH);

	if(byte & 0x02)
		digitalWrite(header->D5,HIGH);

	if(byte & 0x04)
		digitalWrite(header->D6,HIGH);

	if(byte & 0x08)
		digitalWrite(header->D7,HIGH);
	
     
    pulse(header);
 
}

void moveCursor(hd44780 * header, cursorMovement movement)
{
	if(!header)
		return;

	if(movement == CURSOR_RIGTH)
		writeBytes(header, 0x14, 0);
	else if(movement == CURSOR_LEFT)
		writeBytes(header, 0x10, 0);
	else 
		writeBytes(header, 0x02, 0);

}

void printString(hd44780 * header, char * string)
{
	if(!header || !string ||!*string)
		return; 

	int positionInLine = 0;

    int i;	
	for(i = 0; string[i] != '\0' ;i++)
	{
		if(positionInLine == header->rowNumber)
		{
		    writeBytes(header, LCD_DDRAMADDRESS | 0xC0, LCD_COMMAND_MODE); //jump to next line
		    positionInLine = 0;
		}

		if(string[i] == '\n' )
		{
			writeBytes(header, LCD_DDRAMADDRESS | 0xC0, LCD_COMMAND_MODE);
			positionInLine = 0;
		}
		else
		{
			writeBytes(header, string[i], LCD_CHARACTER_MODE);
			positionInLine = positionInLine + 1;
		}
	}

}

void scrollDisplay(hd44780 * header, displayScroll scroll)
{
	if(!header)
		return;

	if(scroll == DISPLAY_SCROLLEFT)
	{
		writeBytes(header,LCD_CURSORSHIFT | LCD_SHIFTDISPLAY | LCD_LEFT , LCD_COMMAND_MODE);
	}
	else if(scroll == DISPLAY_SCROLLRIGTH)
	{
		writeBytes(header,LCD_CURSORSHIFT | LCD_SHIFTDISPLAY | LCD_RIGHT , LCD_COMMAND_MODE);
	}
}

void clearDisplay(hd44780 * header)
{
	if(!header)
		return;

	writeBytes(header, 0x01, 0);
}

void printInt32(hd44780 * header, int val)
{
	if(!header)
		return;

	int i;
	for(i = 0; i < 32; i++)
	{
		if(i == header->rowNumber)
		    writeBytes(header, LCD_DDRAMADDRESS | 0xC0, LCD_COMMAND_MODE); //jump to next line

		if((val>>i) & 0x01)
			writeBytes(header, 0x31, 1);
		else
			writeBytes(header, 0x30, 1);

	}
}

