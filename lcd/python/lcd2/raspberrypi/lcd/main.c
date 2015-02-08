#include "Lcd.h"

int main()
{
	hd44780 header;
	setDefaultHd44780(&header);
	initializeDisplay(&header);
	writeBytes(&header, 0x33, 0); //INIT
	writeBytes(&header, 0x32, 0); //INIT
	writeBytes(&header, 0x28, 0); 
	writeBytes(&header, 0x0C, 0);
	writeBytes(&header, 0x0f, 0);

	clearDisplay(&header);
	moveCursor(&header,CURSOR_HOME);

    //printInt32(&header, 0x00080);

    printString(&header,"Raspberry Pi\nRaspberry Pi");   

    return 0;
}