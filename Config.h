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

#pragma once

#include <EEPROM.h>
#include <IPAddress.h>
#include <Arduino.h>

#define EEWRITE(v) { EEPROM.put(p, v); p += sizeof(v); }
#define EEREAD(v) { EEPROM.get(p, v); p += sizeof(v); }

#define PROGRAM_OFFSET 32	// The PLC program will be stored in EEPROM starting from this address

class StringParse : public String
{
public:
	String Get(String key) {
		int b = this->indexOf(key);
		if (b < 0) return "";
		b = this->indexOf('=', b);
		if (b < 0) return "";
		b++;
		int e = this->indexOf('\r', b);
		if (e < 0) return "";
		return this->substring(b, e);
	}
};


class Config {
public:
	bool CheckCRC(void);
	void UpdateCRC(void);
private:
	uint16_t eeprom_crc(int minus = 0);
};

class IP_Config_t : public Config {
public:
	bool useDHCP = true;
	uint8_t   mac_address[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
	IPAddress local_ip = { 192, 168, 1, 241 };
	IPAddress dns_server = { 192, 168, 1, 1 };
	IPAddress gateway = { 192, 168, 1, 1 };
	IPAddress subnet = { 255, 255, 255, 0 };
	uint32_t  modbus_baudrate = 9600;
	void SaveConfig();
	void LoadConfig();
	void ParseConfig(StringParse & buf);
	static String IP2Ascii(IPAddress ip);
	static void Ascii2IP(String str, IPAddress & ip);
	static String MAC2Ascii(uint8_t * mac);
	static void Ascii2MAC(String str, uint8_t * mac);
	String toXML(void);
private:
	uint16_t version;
	const uint16_t _version = 0x12F8;
};