// 
// 
// 

#include "ldmicro.h"

#define INT_SET_BIT                              1
#define INT_CLEAR_BIT                            2
#define INT_COPY_BIT_TO_BIT                      3
#define INT_SET_VARIABLE_TO_LITERAL              4
#define INT_SET_VARIABLE_TO_VARIABLE             5
#define INT_INCREMENT_VARIABLE                   6
#define INT_SET_VARIABLE_ADD                     7
#define INT_SET_VARIABLE_SUBTRACT                8
#define INT_SET_VARIABLE_MULTIPLY                9
#define INT_SET_VARIABLE_DIVIDE                 10
#define INT_READ_ADC                            11
#define INT_SET_PWM                             12

#define INT_IF_BIT_SET                          50
#define INT_IF_BIT_CLEAR                        51
#define INT_IF_VARIABLE_LES_LITERAL             52
#define INT_IF_VARIABLE_EQUALS_VARIABLE         53
#define INT_IF_VARIABLE_GRT_VARIABLE            54 // obsolete

#define INT_ELSE                                60 + 100
#define INT_END_IF                              61 + 100

#define INT_END_OF_PROGRAM                     255

LDmicro::LDmicro()
{
	ResetProgram();
}

void LDmicro::ResetProgram(void)
{
	ProgramReady = false;
	LoaderState = st_init;
	pc = 0;
	line_length = 0;
	time = 0;
	memset(_Bits, 0, sizeof(_Bits));
	memset(_Integers, 0, sizeof(_Integers));
}

int LDmicro::HexDigit(int c)
{
	if ((c >= '0') && (c <= '9')) {
		return c - '0';
	}
	else if ((c >= 'a') && (c <= 'f')) {
		return 10 + (c - 'a');
	}
	else if ((c >= 'A') && (c <= 'F')) {
		return 10 + (c - 'A');
	}
	return -1;
}

void LDmicro::LoadProgramLine(char *line)
{
	line = strtok(line, "\r\n");
	switch (LoaderState) {
		case st_init:
		{
			if (!strcmp(line, "$$LDcode")) LoaderState = st_bytecode;
			break;
		}
		case st_bytecode:
		{
			if (line[0] == '$') {
				LoaderState = st_end;
				break;
			}
			for (char *t = line; t[0] >= 32 && t[1] >= 32; t += 2) {
				Program[pc++] = HexDigit(t[1]) | (HexDigit(t[0]) << 4);
			}
			break;
		}
		case st_end:
		{	
			//$$cycle 10000 us
			if (strstr(line, "$$cycle")) {
				cycle_ms = atoi(line + 7) / 1000;
				ProgramReady = true;
				D(Serial.println("Program Ready"));
				D(Serial.print("cycle time (ms): "));
				D(Serial.println(cycle_ms));
			}
			break;
		}
	}
}

void LDmicro::LoadProgramLine(char c)
{
	if (line_length < sizeof(line)) line[line_length++] = c;
	if (c == '\n' && line_length > 0) {
		line[line_length] = 0;
		LoadProgramLine(line);
		line_length = 0;
	}
}

void LDmicro::InterpretOneCycle(void)
{
	if (!ProgramReady) return;

	for (int pc = 0;;) {
		D(Serial << "opcode[" << String(pc, HEX) << "]=" << String(Program[pc], HEX) << "\n");
		switch (Program[pc]) {
		case INT_SET_BIT:
			WRITE_BIT(Program[pc+1], 1);
			pc += 2;
			break;

		case INT_CLEAR_BIT:
			WRITE_BIT(Program[pc + 1], 0);
			pc += 2;
			break;

		case INT_COPY_BIT_TO_BIT:
			WRITE_BIT(Program[pc+1], READ_BIT(Program[pc + 2]));
			pc += 3;
			break;

		case INT_SET_VARIABLE_TO_LITERAL:
			WRITE_INT(Program[pc+1], Program[pc + 2] + (Program[pc + 3] << 8));
			pc += 4;
			break;

		case INT_SET_VARIABLE_TO_VARIABLE:
			WRITE_INT(Program[pc+1], READ_INT(Program[pc + 2]));
			pc += 3;
			break;

		case INT_INCREMENT_VARIABLE:
			WRITE_INT(Program[pc+1], READ_INT(Program[pc+1]) + 1);
			pc += 2;
			break;

		case INT_SET_VARIABLE_ADD:
			WRITE_INT(Program[pc+1], READ_INT(Program[pc + 2]) + READ_INT(Program[pc + 3]));
			pc += 4;
			break;

		case INT_SET_VARIABLE_SUBTRACT:
			WRITE_INT(Program[pc + 1], READ_INT(Program[pc + 2]) - READ_INT(Program[pc + 3]));
			pc += 4;
			break;

		case INT_SET_VARIABLE_MULTIPLY:
			WRITE_INT(Program[pc + 1], READ_INT(Program[pc + 2]) * READ_INT(Program[pc + 3]));
			pc += 4;
			break;

		case INT_SET_VARIABLE_DIVIDE:
			if (READ_INT(Program[pc + 3]) != 0) {
				WRITE_INT(Program[pc + 1], READ_INT(Program[pc + 2]) / READ_INT(Program[pc + 3]));
			}
			pc += 4;
			break;

		case INT_SET_PWM:
			pc += 4;
			break;

		case INT_READ_ADC:
			pc += 2;
			break;

		case INT_IF_BIT_SET:
			if (!READ_BIT(Program[pc+1])) pc += Program[pc + 2];
			pc += 3;
			break;

		case INT_IF_BIT_CLEAR:
			if (READ_BIT(Program[pc + 1])) pc += Program[pc + 2];
			pc += 3;
			break;

		case INT_IF_VARIABLE_LES_LITERAL:
			if (!(READ_INT(Program[pc + 1]) < (Program[pc + 2] + (Program[pc + 3] << 8)))) pc += Program[pc + 4];
			pc += 5;
			break;

		case INT_IF_VARIABLE_EQUALS_VARIABLE:
			if (!(READ_INT(Program[pc + 1]) == READ_INT(Program[pc + 2]))) pc += Program[pc + 3];
			pc += 4;
			break;

		case INT_IF_VARIABLE_GRT_VARIABLE:
			if (!(READ_INT(Program[pc + 1]) > READ_INT(Program[pc + 2]))) pc += Program[pc + 3];
			pc += 4;
			break;

		case INT_ELSE:
			pc += Program[pc+1];
			pc += 2;
			break;

		case INT_END_OF_PROGRAM:
			return;

		default:
			Serial.print("Unknown opcode: 0x");
			Serial.print(Program[pc], HEX);
			Serial.println("");
			Serial.print("PC: 0x");
			Serial.print(pc, HEX);
			Serial.println("");
			ProgramReady = false;
			return;
		}
	}
}

void LDmicro::Engine(void)
{
	int count = 10;

	if (time == 0) {
		time = millis();
		return;
	}

	while (ProgramReady && time + cycle_ms < millis() && count-- > 0) {
		unsigned long ts = micros();
		InterpretOneCycle();
		processing_time = micros() - ts;
		time = time + cycle_ms;
#if DEBUG
		while (!Serial.available());
		Serial.read();
#endif
	}
}

void LDmicro::WRITE_BIT(int addr, boolean value)
{
	if (addr < XINTERNAL_OFFSET) digitalWrite(addr, value);
	_Bits[addr] = value;
	D(Serial << "write bit[" << addr << "] " << value << "\n");
}

boolean LDmicro::READ_BIT(int addr)
{
	boolean rc;
	if (addr < XINTERNAL_OFFSET) rc = digitalRead(addr);
	else rc = _Bits[addr];
	D(Serial << "read  bit[" << addr << "] " << rc << "\n");
	return rc;
}

void LDmicro::WRITE_INT(int addr, SWORD value)
{
	if (addr < XINTERNAL_OFFSET) analogWrite(addr, value);
	_Integers[addr] = value;
	D(Serial << "write int[" << addr << "] " << value << "\n");
}

LDmicro::SWORD LDmicro::READ_INT(int addr)
{
	SWORD rc;
	if (addr < XINTERNAL_OFFSET) rc = analogRead(addr);
	else rc = _Integers[addr];
	D(Serial << "read  int[" << addr << "] " << rc << "\n");
	return rc;
}
