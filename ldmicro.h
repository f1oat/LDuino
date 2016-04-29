// ldmicro.h

#ifndef _LDMICRO_h
#define _LDMICRO_h

#include <Modbus.h>

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define MAX_PHYS_PINS		128
#define MAX_INT_RELAYS		64
#define MAX_MODBUS_COILS	64

#include "Streaming.h"

#if DEBUG
#define D(x) (x)
#else
#define D(x)
#endif

class LDmicro {
public:
	LDmicro();
	void ClearProgram(void);
	int HexDigit(int c);
	void SetModbus(Modbus *mb) { this->mb = mb; }
	void LoadProgramLine(char *line);
	void LoadProgramLine(char c);
	void ConfigureModbus(void);
	void InterpretOneCycle(void);
	void Engine(void);
	unsigned long GetTime() { return time; };
	int GetProcessingTime() { return processing_time; };
private:
	typedef unsigned char BYTE;     // 8-bit unsigned
	typedef unsigned short WORD;    // 16-bit unsigned
	typedef signed short SWORD;     // 16-bit signed
	
#define IO_TYPE_PENDING         0
#define IO_TYPE_GENERAL         1
#define IO_TYPE_PERSIST         2
#define IO_TYPE_STRING          3
#define IO_TYPE_RTO             4
#define IO_TYPE_COUNTER         5
#define IO_TYPE_INT_INPUT       6
#define IO_TYPE_DIG_INPUT       7
#define IO_TYPE_DIG_OUTPUT      8
#define IO_TYPE_READ_ADC        9
#define IO_TYPE_UART_TX         10
#define IO_TYPE_UART_RX         11
#define IO_TYPE_PWM_OUTPUT      12
#define IO_TYPE_INTERNAL_RELAY  13
#define IO_TYPE_TON             14
#define IO_TYPE_TOF             15
#define IO_TYPE_MODBUS_CONTACT  16
#define IO_TYPE_MODBUS_COIL     17
#define IO_TYPE_MODBUS_HREG     18

	typedef struct PlcProgramSingleIoTag {
		BYTE    type;
		BYTE    pin;
		BYTE	ModbusSlave;
		WORD	ModbusRegister;
	} Map_IO_t;

	typedef struct IO_Var_s {
		Map_IO_t Map;
		SWORD Value;
	} IO_t;

	void WRITE_BIT(BYTE addr, boolean value);
	boolean READ_BIT(BYTE addr);
	void WRITE_INT(BYTE addr, SWORD value);
	SWORD READ_INT(BYTE addr);

	Modbus *mb;

#define MAX_OPS                 512
#define MAX_VARIABLES           256

	BYTE *Program;
	IO_t *IO;
	int nbProgram;
	int nbIO;

	boolean ProgramReady;
	
	/*
		$$IO
		$$LDcode
		$$cycle 10000 us
	*/

	enum state {st_init, st_LDcode, st_IO, st_cycle} LoaderState;
	state ChangeState(char *line);

	int pc;
	int cycle_ms;
	unsigned long time;
	int processing_time;

	char line[80];
	char line_length;
};


#endif

