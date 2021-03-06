#ifndef LCD_H_
#define LCD_H

typedef enum{FALSE,TRUE} bool;

//Lcd instructions
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_CGRAMADDRESS 0x40
#define LCD_DDRAMADDRESS 0x80

//display entry mode flags
#define LCD_SHIFTENTRY 0x01 //the DDRAM address is incremented upon DDRAM write/read
#define LCD_NOSHIFTENTRY 0x00 //the DDRAM address is decremented upon DDRAM write/read
#define LCD_ENTRYDECREMENT 0x00 //the display is not shifted
#define LCD_ENTRYINCREMENT 0x02 //the the display is shifted left if LCD_SHIFTENTRY is set or right if LCD_NOSHIFTENTRY is set

//display control flags
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_HIDECURSOR 0x00
#define LCD_SHOWCURSOR 0x02
#define LCD_CURSORBLINKON 0x01
#define LCD_CURSORBLINKOFF 0x00

//cursor/display flags
#define LCD_MOVECURSOR 0x00
#define LCD_SHIFTDISPLAY 0x08
#define LCD_LEFT 0x00
#define LCD_RIGHT 0x04

//function set flags
#define LCD_5x10FONT 0x04
#define LCD_5x8FONT 0x00
#define LCD_LINES 0x08
#define LCD_LINE 0x00
#define LCD_8BIT 0x10
#define LCD_4BIT 0x00

typedef struct
{
	int registerSelect;
	int enable;

	int D4;
	int D5;
	int D6;
	int D7;

	int colNumber;
	int rowNumber;
} hd44780;


typedef enum {CURSOR_LEFT,CURSOR_RIGTH,CURSOR_HOME}cursorMovement;

typedef enum {DISPLAY_SCROLLEFT,DISPLAY_SCROLLRIGTH}displayScroll;

typedef enum {LCD_COMMAND_MODE, LCD_CHARACTER_MODE}modes;

void setDefaultHd44780(hd44780 * toDefault);
void initializeDisplay(hd44780 * header);
void writeBytes(hd44780 * header, int byte, int mode);
void moveCursor(hd44780 * header, cursorMovement movement);
void scrollDisplay(hd44780 * header, displayScroll scroll);
void printString(hd44780 * header, char * string);
void clearDisplay(hd44780 * header);
void printInt32(hd44780 * header, int val);


#endif