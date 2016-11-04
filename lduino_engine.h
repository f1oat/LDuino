// Copyright 2016 Frederic Rible
//
// This file is part of LDuino, an Arduino based PLC software compatible with LDmicro.
//
// LDuino is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LDuino is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LDmicro.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _LDUINO_h
#define _LDUINO_h

#include "Modbus.h"
#include "Config.h"
#include "xmlstring.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define MAX_PHYS_PINS		128
#define MAX_INT_RELAYS		64
#define MAX_MODBUS_COILS	64

#define PLC_VERSION 2

#include "Streaming.h"

#if DEBUG
#define D(x) (x)
#else
#define D(x)
#endif

class LDuino_engine : public Config {
public:
	LDuino_engine();
	void SetModbus(Modbus *mb);
	void ClearProgram(void);
	void LoadProgramLine(char c);
	void Engine(void);
	unsigned long GetTime() { return time; };
	void PrintStats(Print & stream);
	int GetType(int pin, signed short * value);
	
	void setPWM(int pin, signed short value);
	void setAnalogInput(int pin, signed short value);
	void toggleDigitalOutput(int pin);
	void toggleDigitalInput(int pin);

	void XML_DumpDigitalPins(xmlstring & str, int first, int last, int offset);
	void XML_DumpAnalogPins(xmlstring & str, int first, int last, int offset);
	void XML_State(Print & stream);
	void SaveConfig();
	bool getProgramReady(void) { return ProgramReady; };
	void ToggleProgramRunning(void) { if (ProgramReady) ProgramRunning = !ProgramRunning; }
	void ToggleIO_Polling(void) { IO_Polling = !IO_Polling; }

	short getCycleMS() { return cycle_ms; }

private:
	void LoadProgramLine(char *line);	
	void ConfigureModbus(void);
	void InterpretOneCycle(void);
	int HexDigit(int c);
	void LoadConfig();
	void PrintPin(Print & stream, char prefix, int index, int value);

	typedef unsigned char BYTE;     // 8-bit unsigned
	typedef unsigned short WORD;    // 16-bit unsigned
	typedef signed short SWORD;     // 16-bit signed
	

#define XIO_TYPE_PENDING		 0
#define XIO_TYPE_DIG_INPUT       1
#define XIO_TYPE_DIG_OUTPUT      2
#define XIO_TYPE_READ_ADC        3
#define XIO_TYPE_PWM_OUTPUT      4
#define XIO_TYPE_MODBUS_CONTACT  5
#define XIO_TYPE_MODBUS_COIL     6
#define XIO_TYPE_MODBUS_HREG     7


	typedef struct PlcProgramSingleIoTag {
		BYTE    type;
		BYTE    pin;
		BYTE	ModbusSlave;
		WORD	ModbusOffset;
	} IO_t;

	void WRITE_BIT(BYTE addr, boolean value);
	boolean READ_BIT(BYTE addr);
	void WRITE_INT(BYTE addr, SWORD value);
	SWORD READ_INT(BYTE addr);
	void WRITE_PWM(BYTE addr, SWORD value);
	void READ_ADC(BYTE addr);

	Modbus *mb;

	BYTE version;
	SWORD cycle_ms;
	SWORD nbProgram;
	BYTE nbIO;
	BYTE total_nbIO;

	BYTE *Program;
	IO_t *IO;
	SWORD *Values;

	SWORD EEPROM_ProgramLen;

	boolean ProgramRunning;
	boolean ProgramReady;
	boolean IO_Polling;	// true if physical I/O are polled, false for simulation mode

	/*
		$$IO
		$$LDcode
		$$cycle 10000 us
	*/

	enum state {st_init, st_LDcode, st_IO, st_cycle, st_error} LoaderState;
	state ChangeState(char *line);

	int pc;
	unsigned long time;
	unsigned long processing_time;

	char line[80];
	char line_length;
};


#endif

