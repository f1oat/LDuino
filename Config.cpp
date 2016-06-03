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
	UpdateCRC();
	Serial << "IP Config length " << p << '\n';
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
}
