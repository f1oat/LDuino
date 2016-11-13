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

#include <Streaming.h>
#include <TextFinder.h>
#include <SD.h>
#include <Flash.h>
#include <avr/wdt.h>
#include <TimerOne.h>

#define noPinDefs         // Disable default pin definitions (X0, X1, ..., Y0, Y1, ...)

#include <SPI.h>

#ifdef CONTROLLINO_MAXI
#include <Ethernet.h>
#include <Controllino.h>
#else
#include <EthernetV2_0.h>
#define SDCARD_CS 4
#endif
#include <EEPROM.h>

#include "Modbus.h"
#include "ModbusIP.h"
#include "ModbusRelay.h"
#include "plcweb.h"
#include "lduino_engine.h"
#include "Config.h"

//ModbusIP object
ModbusIP mb;

// LDmicro Ladder interpreter
LDuino_engine lduino;
IP_Config_t IP_Config;;
bool doReset = false;

#ifdef CONTROLLINO_MAXI
void switch_txrx(ModbusRelay::txrx_mode mode)
{
	switch (mode) {
	case ModbusRelay::tx:
		Controllino_SwitchRS485DE(1);
		Controllino_SwitchRS485RE(1);
		break;
	case ModbusRelay::rx:
		Controllino_SwitchRS485DE(0);
		Controllino_SwitchRS485RE(0);
		break;
	default:	
		Controllino_SwitchRS485DE(0);
		Controllino_SwitchRS485RE(1);
		break;
	}
}
#endif

void setup_MODBUS()
{
	mb.config(); 	//Config Modbus IP
#ifdef CONTROLLINO_MAXI
	Controllino_RS485Init();
	mb.configRelay(&Serial3, IP_Config.modbus_baudrate, SERIAL_8N1, switch_txrx);
#endif
	lduino.SetModbus(&mb);
}


void pollPLC()
{
	lduino.Engine();
}

void setup() {
	Serial.begin(115200);
	Serial << F("Setup\n");

	IP_Config.LoadConfig();

#ifndef CONTROLLINO_MAXI
	pinMode(SDCARD_CS, OUTPUT);
	digitalWrite(SDCARD_CS, HIGH);//Deselect the SD card
#endif

	if (IP_Config.useDHCP) {
		Serial << F("Trying DHCP ...\n");
		if (!Ethernet.begin(IP_Config.mac_address)) {
			Serial << F("DHCP failure\n");
		}
		else {
			Serial << F("DHCP ok\n");
			IP_Config.local_ip = Ethernet.localIP();
			IP_Config.subnet = Ethernet.subnetMask();
			IP_Config.dns_server = Ethernet.dnsServerIP();
			IP_Config.gateway = Ethernet.gatewayIP();
		}
	}

	if (!IP_Config.useDHCP || Ethernet.localIP() == INADDR_NONE) {
		Ethernet.begin(IP_Config.mac_address, IP_Config.local_ip, IP_Config.dns_server, IP_Config.gateway, IP_Config.subnet);
	}

	Serial << F("IP: ") << Ethernet.localIP() << '\n';

	customIO();			// Setup inputs and outputs for Controllino PLC
	setup_MODBUS();
	setup_PLC_Web();	// Web server init

	Timer1.initialize(lduino.getCycleMS() * 1000L);
	Timer1.attachInterrupt(pollPLC);

	Serial << F("PLC ready\n");
} 

void loop() {
	mb.task();
	poll_PLC_Web();
	//lduino.Engine();
	if (doReset) {
		Serial << "Reset requested\n";
		delay(500);
		Timer1.stop();
		noInterrupts();
#ifdef CONTROLLINO_MAXI
		pinMode(45, OUTPUT);
		digitalWrite(45, 0);	// X2 connector bridge between pin 6 and 11
#else
		wdt_disable();
		wdt_enable(WDTO_1S);
#endif
		doReset = false;
	}
}
