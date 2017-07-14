#include <Arduino.h>
#include "sysinfo.h"

int sysinfo::unusedRam()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// Wipe RAM between heap and stack with 0x55 pattern

void sysinfo::wipeRam()
{
	extern int __heap_start, *__brkval;
	int v;
	for (byte *ptr = (byte *)(__brkval == 0 ? &__heap_start : __brkval); ptr < (byte *)&v; ptr++) {
		*ptr = 0x55;

	}
}