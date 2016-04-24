// plcweb.h

#ifndef _PLCWEB_h
#define _PLCWEB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

void setup_PLC_Web();
void poll_PLC_Web();
#endif

