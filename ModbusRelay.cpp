#include "ModbusRelay.h"

ModbusRelay::ModbusRelay() 
{
	this->_SerialInProgress = false;
	this->client = NULL;
	this->ModbusTimeout_ms = 100;
}

void ModbusRelay::configRelay(HardwareSerial* port, long baud, u_int format, void(*_switch_txrx)(txrx_mode)) 
{
	this->_port = port;
	port->begin(baud, format);
	this->_switch_txrx = _switch_txrx;

	if (baud > 19200) {
		_t15 = 750;
		_t35 = 1750;
	}
	else {
		_t15 = 15000000 / baud; // 1T * 1.5 = T1.5
		_t35 = 35000000 / baud; // 1T * 3.5 = T3.5
	}
	Serial.print(F("t_15="));
	Serial.println(_t15);
	Serial.print(F("t_35="));
	Serial.println(_t35);
}

void ModbusRelay::pollSerial()
{
	if (!_SerialInProgress) return;

	if (micros() > _timeoutTransaction) {
		Serial.print(F("RS485: timeout "));
		Serial.println(_port->available());
		Serial.println(_len);
		Serial.println(_timeoutFrame);
		Serial.println(_timeoutTransaction);
		Serial.println(micros());
		_len = 0;
		_SerialInProgress = false;
		while (_port->available()) _port->read();
		// Switch off receiver
		if (_switch_txrx) _switch_txrx(off);
		return;
	}

	if (_port->available() > _len) {	// We have received new data
		_len = _port->available();
		_timeoutFrame = micros() + _t35;
	}
	else if (_len > 1 && micros() > _timeoutFrame) {
		if (_len >= 3) {
			byte i;
			_rxid = _port->read();
			_len--;
			if (_rxid == 0 && _len > 0) {	// Remove random parasitic zero after remote switch TX on
				_rxid = _port->read();
				_len--;
			}
			_frame = (byte*)malloc(_len);
			for (i = 0; i < _len; i++) {
				_frame[i] = _port->read();
			}

			if (client) RX();
			free(_frame);
		}
		else {
			Serial.print(F("RS485: short frame "));
			Serial.println(_len);
		}
		while (_port->available()) _port->read();
		_len = 0;
		_SerialInProgress = false;
		// Switch off receiver
		if (_switch_txrx) _switch_txrx(off);
	}
}

void ModbusRelay::TX(EthernetClient client, byte MBAP[], byte *frame, byte  len)
{
	if (_SerialInProgress) return;	// We cannot have two transactions at the same time

	Serial.print("RS485 TX ID=");
	Serial.print(MBAP[6]);
	Serial.print(" F=");
	Serial.println(frame[0]);
	//Serial.println(len);

	memcpy(_MBAP, MBAP, 7);
	this->client = client;
	
	// Store function cocde rto report exception if neeeded
	this->_fc = frame[0];

	// Switch to TX mode
	if (_switch_txrx) _switch_txrx(tx);;

	//Send slaveId
	_port->write(MBAP[6]);

	//Send PDU
	byte i;
	for (i = 0; i < len; i++) {
		_port->write(frame[i]);
	}

	//Send CRC
	word crc = calcCrc(MBAP[6], frame, len);
	_port->write(crc >> 8);
	_port->write(crc & 0xFF);

	_port->flush();
	//delayMicroseconds(_t35);

	// Switch to RX mode
	if (_switch_txrx) _switch_txrx(rx);

	_len = 0;
	_timeoutTransaction = micros() + ModbusTimeout_ms * 1000L;
	_timeoutFrame = 0;
	_SerialInProgress = true;
}

bool ModbusRelay::RX()
{
	Serial.print(F("RS485 RX "));
	Serial.println(_len);

	//Last two bytes = crc
	u_int crc = ((_frame[_len - 2] << 8) | _frame[_len - 1]);

	//CRC Check
	if (crc != calcCrc(_rxid, _frame, _len - 2)) {
		Serial.print(F("*** RS485: CRC error "));
		for (byte i=0; i<_len; i++) {
			Serial.print(_frame[i]);
			Serial.print(' ');
		}
		Serial.print('\n');
		// Report exception "slave device failure"
		_frame[0] = _fc | 0x80;
		_frame[1] = 0x04;
		_len = 2;
	}
	else {
		_MBAP[6] = _rxid;
		_len -= 2; // remove CRC
	}

	//MBAP
	_MBAP[4] = (_len + 1) >> 8;     //_len+1 for last byte from MBAP
	_MBAP[5] = (_len + 1) & 0x00FF;

	byte sendbuffer[7 + _len];

	for (int i = 0; i < 7; i++) {
		sendbuffer[i] = _MBAP[i];
	}
	//PDU Frame
	for (int i = 0; i < _len; i++) {
		sendbuffer[i + 7] = _frame[i];
	}
	client.write(sendbuffer, _len + 7);
	return true;
}

