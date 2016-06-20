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

#include <Streaming.h>>
#include "Config.h"
#include "xmlstring.h"

// Code coming from http://www.ccontrolsys.com/w/How_to_Compute_the_Modbus_RTU_Message_CRC

uint16_t Config::eeprom_crc(int minus)
{
	uint16_t crc = ~0L;

	for (int index = 0; index < EEPROM.length() - minus; ++index) {
		crc ^= EEPROM[index];          // XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			}
			else                            // Else LSB is not set
				crc >>= 1;                    // Just shift right
		}                    // Just shift right
	}
	return crc;
}

bool Config::CheckCRC(void)
{
	return eeprom_crc(0) == 0;
}

void Config::UpdateCRC(void)
{
	uint16_t crc = eeprom_crc(2);
	int p = EEPROM.length() - 2;
	EEPROM[p++] = crc & 0xFF;
	EEPROM[p++] = crc >> 8;
}

void IP_Config_t::SaveConfig()
{
	int p = 0;
	version = _version;
	EEWRITE(version);
	EEWRITE(useDHCP);
	EEWRITE(mac_address);
	EEWRITE((uint32_t)local_ip);
	EEWRITE((uint32_t)dns_server);
	EEWRITE((uint32_t)gateway);
	EEWRITE((uint32_t)subnet);
	EEWRITE(modbus_baudrate);
	UpdateCRC();
}

void IP_Config_t::LoadConfig()
{
	int p = 0;
	uint32_t buf;

	if (!CheckCRC()) return;
	EEREAD(version);
	if (version != _version) return;
	
	EEREAD(useDHCP);
	EEREAD(mac_address);
	EEREAD(buf);
	local_ip = buf;
	EEREAD(buf);
	dns_server = buf;
	EEREAD(buf);
	gateway = buf;
	EEREAD(buf);
	subnet = buf;
	EEREAD(modbus_baudrate);
}

void IP_Config_t::ParseConfig(StringParse &buf)
{
	useDHCP = buf.Get(F("useDHCP")) == "true" ? true : false;
	Ascii2MAC(buf.Get(F("mac")), mac_address);
	Ascii2IP(buf.Get(F("ip")), local_ip);
	Ascii2IP(buf.Get(F("subnet")), subnet);
	Ascii2IP(buf.Get(F("gateway")), gateway);
	Ascii2IP(buf.Get(F("dns")), dns_server);
	modbus_baudrate = buf.Get(F("modbus_baudrate")).toInt();
}

String IP_Config_t::IP2Ascii(IPAddress ip)
{
	char buf[16];
	sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	return String(buf);
}

void IP_Config_t::Ascii2IP(String str, IPAddress &ip)
{
	sscanf(str.c_str(), "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
}

String IP_Config_t::MAC2Ascii(uint8_t *mac)
{
	char buf[18];
	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return String(buf);
}

void IP_Config_t::Ascii2MAC(String str, uint8_t *mac)
{
	sscanf(str.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
}

String IP_Config_t::toXML(void)
{
	xmlstring str;
	str += F("<?xml version = \"1.0\" ?>\n");
	str += F("<config>\n");
	str.catTag(F("useDHCP"), useDHCP);
	str.catTag(F("mac"), MAC2Ascii(mac_address));
	str.catTag(F("ip"), IP2Ascii(local_ip));
	str.catTag(F("subnet"), IP2Ascii(subnet));
	str.catTag(F("dns"), IP2Ascii(dns_server));
	str.catTag(F("gateway"), IP2Ascii(gateway));
	str.catTag(F("modbus_baudrate"), String(modbus_baudrate));
	str += F("</config>\n");
	return str;
}