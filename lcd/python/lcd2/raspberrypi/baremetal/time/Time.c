#include "Time.h"
#include "memory/MemoryOp.h"
#include "main/Common.h"

void delayMicro(unsigned int time)
{
	unsigned int base = read(FREE_RUNNING_TIMER_BASE);

    while(TRUE)
    {
        unsigned int current = read(FREE_RUNNING_TIMER_BASE);

        if((current - base) >= time) 
            break;
    }
}

void delaySeconds(unsigned int time)
{
	time = time * 1000000;

	unsigned int base = read(FREE_RUNNING_TIMER_BASE);

    while(TRUE)
    {
        unsigned int current = read(FREE_RUNNING_TIMER_BASE);

        if((current - base) >= time) 
            break;
    }
}