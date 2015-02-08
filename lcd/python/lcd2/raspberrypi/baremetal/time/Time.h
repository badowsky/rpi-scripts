#ifndef TIME_H_
#define TIME_H_

#define FREE_RUNNING_TIMER_BASE 0x20003004

void delayMicro(unsigned int time);
void delaySeconds(unsigned int time);

#endif