#pragma once

#include <Arduino.h>

#ifdef CONTROLLINO_MAXI
#include <Ethernet.h>
#else
#include <EthernetV2_0.h>
#endif

#include "ModbusSerial.h"

class ModbusRelay : public ModbusSerial {
private:
	unsigned int ModbusTimeout_ms;	// Modbus timeout for Serial transactions in ms

	bool _SerialInProgress; // True when a transaction is in progress over the serial port*
	unsigned long _timeoutTransaction;
	unsigned long _timeoutFrame;

	byte _MBAP[7];
	byte *_frame;
	byte  _len;
	byte _tcp_len;
	EthernetClient client;
	byte _fc;	// Function code of on-going transaction
	byte _rxid;	// Received slaveID

	bool RX();

	short stats_transaction;
	short stats_crc_error;
	short stats_timeout;
	short stats_short_frame;
public:
	
	ModbusRelay();
	typedef enum { off, tx, rx } txrx_mode;
	void(*_switch_txrx)(txrx_mode);
	
	void configRelay(HardwareSerial * port, long baud, u_int format, void(*_switch_txrx)(txrx_mode));
	void TX(EthernetClient client, byte MBAP[], byte * frame, byte len);
	bool pollSerial();
	void pollTCP();

	String DumpStats();
};
