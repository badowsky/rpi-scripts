#include "Postman.h"
#include "MemoryOp.h"

bool writeOnMailbox(int data, channels channel)
{
	if(channel > PTAGS_VC)
		return FALSE;

	while(read(MAILBOX_BASE + MAILBOX_STATUS_OFFSET) & MAILBOX_FULL)
	{
		/*wait*/
		nop();
	}

	store(MAILBOX_BASE + MAILBOX_WRITE_OFFSET,data<<4 | channel);

	return TRUE;
}
int readFromMailbox(channels channel)
{
	if(channel > PTAGS_VC)
		return FALSE;

	while(read(MAILBOX_BASE + MAILBOX_STATUS_OFFSET) & MAILBOX_EMPTY)
	{
		/*wait*/
		nop();
	}

	unsigned int data = read(MAILBOX_BASE * MAILBOX_READ_OFFSET);

	/*check the channel*/

	/*output*/

	return 0;
}