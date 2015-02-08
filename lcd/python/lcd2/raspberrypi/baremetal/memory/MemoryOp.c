#include "MemoryOp.h"

void store(unsigned int memAddress, int value)
{
	asm volatile("str r1,[r0]":::"memory");
}

int read(unsigned int memAddress)
{
	asm volatile("ldr r0,[r0]"::: "memory");
}