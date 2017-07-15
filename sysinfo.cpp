#include <Arduino.h>
#include "sysinfo.h"

#define USE_RTOS 1
#ifdef USE_RTOS
#include <Arduino_FreeRTOS.h>
#include <task.h>
#endif

int sysinfo::freeRam()
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

// Measure stack usage

int sysinfo::stackUsage()
{
	extern int __stack;
	extern int __heap_start, *__brkval;

	short *heap_top = (short *)(__brkval == 0 ? &__heap_start : __brkval);
	short *ptr = (short *)&__stack;

	int count = 0;
	while (*ptr != 0x5555 && ptr >= heap_top) {
		ptr--;
		count += 2;
	}

	return count;
}

// Measure unused Ram

int sysinfo::unusedRam()
{
	int count = 0;
	for (short *ptr = 0; (int)ptr <= RAMEND; ptr++) {
		if (*ptr == 0x5555) count += 2;
	}
	return count;
}

#ifdef USE_RTOS
String sysinfo::DumpRTOS(void)
{
	TaskStatus_t *pxTaskStatusArray; 
	UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();
	String result;
	char line[32];

	pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));
	if (pxTaskStatusArray == NULL) return "";

	uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, NULL);

	result += F("Task        |Free stack\n");
	result += F("------------------------\n");
	for (byte x = 0; x < uxArraySize; x++)
	{
		sprintf(line, "%-12s|%10d\n", pxTaskStatusArray[x].pcTaskName, pxTaskStatusArray[x].usStackHighWaterMark);
		result += line;
	}
		
	vPortFree(pxTaskStatusArray);
	return result;
}
#endif