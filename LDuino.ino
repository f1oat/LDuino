#include <Streaming.h>
#include <TextFinder.h>
#include <SD.h>
#include <Flash.h>
#include <avr/wdt.h>
#include <TimerOne.h>

#define noPinDefs         // Disable default pin definitions (X0, X1, ..., Y0, Y1, ...)

#include <SPI.h>
#include <Ethernet.h>
#include <Controllino.h>
#include <EEPROM.h>

#include "Modbus.h"
#include "ModbusIP.h""
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
#endif

void setup_MODBUS()
{
	mb.config(); 	//Config Modbus IP
#ifdef CONTROLLINO_MAXI
	Controllino_RS485Init();
	mb.configRelay(&Serial3, 9600, SERIAL_8N1, switch_txrx);
#endif
	lduino.SetModbus(&mb);
}


void pollPLC()
{
	lduino.Engine();
}

void setup() {
	Serial.begin(115200);

	IP_Config.LoadConfig();

	if (IP_Config.useDHCP) {
		Serial << F("Trying DHCP ...\n");
		if (!Ethernet.begin(IP_Config.mac_address)) {
			Serial << F("DHCP failure\n");
		}
		else {
			IP_Config.local_ip = Ethernet.localIP();
			IP_Config.subnet = Ethernet.subnetMask();
			IP_Config.dns_server = Ethernet.dnsServerIP();
			IP_Config.gateway = Ethernet.gatewayIP();
		}
	}

	if (!IP_Config.useDHCP || Ethernet.localIP() == INADDR_NONE) {
		Ethernet.begin(IP_Config.mac_address, IP_Config.local_ip, IP_Config.dns_server, IP_Config.gateway, IP_Config.subnet);
	}

	customIO();			// Setup inputs and outputs for Controllino PLC
	setup_MODBUS();
	setup_PLC_Web();	// Web server init

	Timer1.initialize(10000);	//TODO: use cycle time defined in XINT program
	Timer1.attachInterrupt(pollPLC);

	Serial << F("PLC ready\n");
	Serial << F("IP: ") << Ethernet.localIP() << '\n';
} 

void loop() {
	mb.task();
	poll_PLC_Web();
	//lduino.Engine();
	if (doReset) {
		Serial << "Reset requested\n";
		wdt_enable(WDTO_1S);
		for (;;);
	}
}
