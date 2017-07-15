/*
    ModbusIP.h - Header for Modbus IP Library
    Copyright (C) 2015 André Sarmento Barbosa
*/
#include <Arduino.h>
#include <SPI.h>

#ifdef CONTROLLINO_MAXI
#include <Ethernet.h>
#else
#include <EthernetV2_0.h>
#endif

#include "Modbus.h"
#include "ModbusRelay.h"

#ifndef MODBUSIP_H
#define MODBUSIP_H

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200

#define TCP_KEEP_ALIVE

//#define TCP_KEEP_ALIVE

class ModbusIP : public Modbus {
    public:			
		ModbusIP();
		void config();
		void config(uint8_t *mac);
		void config(uint8_t *mac, IPAddress ip);
		void config(uint8_t *mac, IPAddress ip, IPAddress dns);
		void config(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway);
		void config(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet);
		void configRelay(HardwareSerial* port, long baud, u_int format, void (*_switch_txrx)(ModbusRelay::txrx_mode));
		void pollTCP();
		void pollSerial();
    private:
        EthernetServer _server;
        byte _MBAP[7];
		EthernetClient client;
		
		// Additional objects for Serial port relaying of traffic not managed locally
		byte  _slaveId;; // Modbus address locally managed
		word calcCrc(byte address, byte* pduframe, byte pdulen);
		 
		ModbusRelay _relay;
	};

#endif //MODBUSIP_H

