/*
    ModbusSerial.h - Header for ModbusSerial Library
    Copyright (C) 2014 André Sarmento Barbosa
*/
#include <Arduino.h>

#include "Modbus.h"

#ifndef MODBUSSERIAL_H
#define MODBUSSERIAL_H

//#define USE_SOFTWARE_SERIAL

#ifdef USE_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
#endif

class ModbusSerial : public Modbus {
    protected:
        Stream* _port;
        long  _baud;
        u_int _format;
        int   _txPin;
        unsigned int _t15; // inter character time out
        unsigned int _t35; // frame delay
        byte  _slaveId;
        word calcCrc(byte address, byte* pduframe, byte pdulen);
    public:
        ModbusSerial();
        bool setSlaveId(byte slaveId);
        byte getSlaveId();
        bool config(HardwareSerial* port, long baud, u_int format, int txPin=-1);
        #ifdef USE_SOFTWARE_SERIAL
        bool config(SoftwareSerial* port, long baud, int txPin=-1);
        #endif
        #ifdef __AVR_ATmega32U4__
        bool config(Serial_* port, long baud, u_int format, int txPin=-1);
        #endif
        void task();
        bool receive(byte* frame);
        bool sendPDU(byte* pduframe);
        bool send(byte* frame);
};

#endif //MODBUSSERIAL_H
