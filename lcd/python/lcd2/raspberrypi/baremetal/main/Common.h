#ifndef COMMON_H_
#define COMMON_H_

#define NULL 0
typedef enum boolean{FALSE,TRUE} bool;

static inline void nop()
{
	asm volatile("mov r0, r0");
}

#endif