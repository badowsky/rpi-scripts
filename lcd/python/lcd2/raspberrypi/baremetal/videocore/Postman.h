#ifndef POSTMAN_H_
#define POSTMAN_H_

#include "Common.h"

#define MAILBOX_BASE 0x2000B880
#define MAILBOX_READ_OFFSET 0x0
#define MAILBOX_POLL_OFFSET 0x10
#define MAILBOX_SENDER_OFFSET 0x14
#define MAILBOX_STATUS_OFFSET 0x18
#define MAILBOX_CONFIG_OFFSET 0x1C
#define MAILBOX_WRITE_OFFSET 0x20

#define MAILBOX_FULL 0x80000000
#define MAILBOX_EMPTY 0x40000000

typedef enum{
	POWER_MANAGEMENT,
	FRAMEBUFFER,
	VUART,
	VCHIQ,
	LEDS,
	BUTTONS,
	TOUCH_SCREEN,
	UNDEF,
	PTAGS_ARM,
	PTAGS_VC
} channels;

bool writeOnMailbox(int data, channels channel);
int readFromMailbox(channels channel);


#endif
