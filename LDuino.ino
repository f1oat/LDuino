#include <Streaming.h>
#include "ldmicro.h"
#include "ldmicro.h"
#include <TextFinder.h>
#include <SD.h>
#include <Flash.h>
#define noPinDefs         // Disable default pin definitions (X0, X1, ..., Y0, Y1, ...)
#include <plcLib.h>       // Load the PLC library

#include <SPI.h>
#include <Ethernet.h>
#include <Modbus.h>
#include <ModbusIP.h>
#include <Controllino.h>

#include "plcweb.h"
#include "ldmicro.h"

/* Programmable Logic Controller Library for the Arduino and Compatibles

Controllino Maxi PLC - Use of default pin names and numbers
Product information: http://controllino.cc

Connections:
Inputs connected to pins A0 - A9, plus interrupts IN0 and IN1
Digital outputs connected to pins D0 to D11
Relay outputs connected to pins R0 to R9

Software and Documentation:
http://www.electronics-micros.com/software-hardware/plclib-arduino/

*/

// Pins A0 - A9 are configured automatically

// Interrupt pins
const int IN0 = 18;
const int IN1 = 19;

const int D0 = 2;
const int D1 = 3;
const int D2 = 4;
const int D3 = 5;
const int D4 = 6;
const int D5 = 7;
const int D6 = 8;
const int D7 = 9;
const int D8 = 10;
const int D9 = 11;
const int D10 = 12;
const int D11 = 13;

const int R0 = 22;
const int R1 = 23;
const int R2 = 24;
const int R3 = 25;
const int R4 = 26;
const int R5 = 27;
const int R6 = 28;
const int R7 = 29;
const int R8 = 30;
const int R9 = 31;

//ModbusIP object
ModbusIP mb;

// LDmicro Ladder interpreter
LDmicro ldmicro;

void switch_txrx(ModbusIP::txrx_mode mode)
{
	switch (mode) {
	case ModbusIP::tx:
		Controllino_SwitchRS485DE(1);
		Controllino_SwitchRS485RE(1);
		break;
	case ModbusIP::rx:
		Controllino_SwitchRS485DE(0);
		Controllino_SwitchRS485RE(0);
		break;
	default:	
		Controllino_SwitchRS485DE(0);
		Controllino_SwitchRS485RE(1);
		break;
	}
}

void setup_MODBUS()
{
	
	byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // The media access control (ethernet hardware) address for the shield
	byte ip[] = { 192, 168, 1, 241 }; // The IP address for the shield
	mb.config(mac, ip); 	//Config Modbus IP
	Controllino_RS485Init();
	mb.configRelay(&Serial3, 9600, SERIAL_8N1, switch_txrx);
	ldmicro.SetModbus(&mb);
}

void setup() {
	Serial.begin(115200);
	customIO();			// Setup inputs and outputs for Controllino PLC
	setup_MODBUS();
	setup_PLC_Web();	// Web server init
	Serial.println("PLC ready");
} 

// Variables:
unsigned long AUX0 = 0;                 // Pulse low timer variable
unsigned long AUX1 = 0;                 // Pulse high timer variable
unsigned int enable = 1;

void loop() {
	mb.task();
	poll_PLC_Web();
	ldmicro.Engine();

	in(enable);
	timerCycle(AUX0, 200, AUX1, 100);
	out(D11);

}
