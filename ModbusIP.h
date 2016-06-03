/*
    ModbusIP.h - Header for Modbus IP Library
    Copyright (C) 2015 André Sarmento Barbosa
*/
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

#include "Modbus.h"

#ifndef MODBUSIP_H
#define MODBUSIP_H

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200

#define TCP_KEEP_ALIVE

//#define TCP_KEEP_ALIVE

class ModbusIP : public Modbus {
    public:
		typedef enum {off, tx, rx} txrx_mode;
			
		ModbusIP();
		void config();
		void config(uint8_t *mac);
		void config(uint8_t *mac, IPAddress ip);
		void config(uint8_t *mac, IPAddress ip, IPAddress dns);
		void config(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway);
		void config(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet);
		void configRelay(HardwareSerial* port, long baud, u_int format, void (*_switch_txrx)(txrx_mode));
		void task();
    private:
        EthernetServer _server;
        byte _MBAP[7];
		EthernetClient client;
		
		// Additional objects for Serial port relaying of traffic not managed locally
		byte  _slaveId;; // Modbus address locally managed
		word calcCrc(byte address, byte* pduframe, byte pdulen);
        Stream* _port;
        long  _baud;
        u_int _format;
        int   _txPin;
        unsigned int _t15; // inter character time out
        unsigned int _t35; // frame delay
		unsigned int ModbusTimeout_ms;	// Modbus timeout for Serial transactions in ms

		bool _SerialInProgress; // True when a transaction is in progress over the serial port*
		unsigned long _timeoutTransaction;
		unsigned long _timeoutFrame;
		
		void (*_switch_txrx)(txrx_mode);
		 
		void pollTCP();
		void pollSerial();		
		bool TCP2Serial_Relay();
		bool Serial2TCP_Relay();
	};

#endif //MODBUSIP_H

