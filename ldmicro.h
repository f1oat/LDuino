// ldmicro.h

#ifndef _LDMICRO_h
#define _LDMICRO_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define XINTERNAL_OFFSET						128 // Direct Arduino pin numbers are coded from 0 to XINTERNAL_OFFSET-1, above are local variables

#include "Streaming.h"

#if DEBUG
#define D(x) (x)
#else
#define D(x)
#endif

class LDmicro {
public:
	LDmicro();
	void ResetProgram(void);
	int HexDigit(int c);
	void LoadProgramLine(char *line);
	void LoadProgramLine(char c);
	void InterpretOneCycle(void);
	void Engine(void);
	unsigned long GetTime() { return time; };
	int GetProcessingTime() { return processing_time; };
private:
	typedef unsigned char BYTE;     // 8-bit unsigned
	typedef unsigned short WORD;    // 16-bit unsigned
	typedef signed short SWORD;     // 16-bit signed

	void WRITE_BIT(int addr, boolean value);
	boolean READ_BIT(int addr);
	void WRITE_INT(int addr, SWORD value);
	SWORD READ_INT(int addr);

#define MAX_OPS                 512
#define MAX_VARIABLES           256
#define MAX_INTERNAL_RELAYS     256

	BYTE Program[MAX_OPS];
	boolean ProgramReady;
	enum state {st_init, st_bytecode, st_end, st_error} LoaderState;
	int pc;
	int cycle_ms;
	unsigned long time;
	int processing_time;

	SWORD _Integers[256];
	boolean _Bits[256];
	char line[80];
	char line_length;
};


#endif

