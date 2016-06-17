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

#include "Arduino.h"

#ifdef CONTROLLINO_MAXI
#include <Ethernet.h>
#else
#include <EthernetV2_0.h>
#endif

#include <Flash.h>

#include "TinyWebServer.h"
#include "plcweb.h"
#include "CircularBuffer.h"
#include "lduino_engine.h"
#include "Config.h"

extern LDuino_engine lduino;
extern IP_Config_t IP_Config;
extern bool doReset;

static boolean index_handler(TinyWebServer& web_server);
static boolean config_handler(TinyWebServer& web_server);
static boolean reboot_handler(TinyWebServer& web_server);
static boolean reboot_confirm_handler(TinyWebServer& web_server);

FLASH_STRING(index_html,
	"<html><body><pre>"
	"<b>LDuino PLC - (C) 2016 Frederic Rible (frible@teaser.fr)</b>\n"
	"%status"
	"<hr>"
	"<table><tr>"
	
	"<td valign='top'><form action = '/config' enctype = 'text/plain' method = 'post'>"
	"<table>"
	"<tr><td>Use DHCP</td><td><input type = 'checkbox' name = 'useDHCP' value = 'true' %useDHCP></td></tr>"
	"<tr><td>MAC</td><td><input type = 'text' name = 'mac' size = '18' value = '%mac'></td></tr>"
	"<tr><td>IP</td><td><input type = 'text' name = 'ip' size = '18' value = '%ip'></td></tr>"
	"<tr><td>Subnet</td><td><input type = 'text' name = 'subnet' size = '18' value = '%subnet'></td></tr>"
	"<tr><td>Gateway</td><td><input type = 'text' name = 'gateway' size = '18' value = '%gateway'></td></tr>"
	"<tr><td>DNS</td><td><input type = 'text' name = 'dns' size = '18' value = '%dns'></td></tr>"
	"<tr><td>MODBUS Baudrate</td><td><input type = 'text' name = 'modbus_baudrate' size = '18' value = '%modbus_baudrate'></td></tr>"
	"<tr><td><input type = 'submit' value = 'Save config'></td></tr>"
	"</table>"
	"</form></td>"
	
	"<td valign='top'><form action = '/upload' enctype = 'multipart/form-data' method = 'post'>"
	"<table>"
	"<tr><td><input type = 'file' name = 'datafile' size = '40'></td></tr>"
	"<tr><td><input type = 'submit' value = 'Upload LDmicro XINT file'></td></tr>"
	"</table>"
	"</form></td>"

	"</tr></table>"
	"<a href='/'>Refresh</a>             <a href = '/reboot'>Reboot</a>"
	"<hr>"
	);

FLASH_STRING(reboot_confirm_html,
	"<html>"
	"<head><meta http-equiv='refresh' content='15; url = http://%ip/'></head>"
	"<body>Rebooting ...</body>"
	"</html>"
);

static TinyWebServer::PathHandler handlers[] = {
	{ "/upload", TinyWebServer::POST, &TinyWebPutHandler::put_handler },
	{ "/config", TinyWebServer::POST, &config_handler },
	{ "/reboot", TinyWebServer::GET, &reboot_handler },
	{ "/reboot_confirm", TinyWebServer::GET, &reboot_confirm_handler },
	{ "/" "*", TinyWebServer::GET, &index_handler },
	{ NULL },
};

static const char* headers[] = {
	"Content-Length",
	"Content-Type",
	NULL
};

static TinyWebServer web = TinyWebServer(handlers, headers);

static String IP2Ascii(IPAddress ip)
{
	char buf[16];
	sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	return String(buf);
}

static void Ascii2IP(String str, IPAddress &ip)
{
	sscanf(str.c_str(), "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
}

static String MAC2Ascii(uint8_t *mac)
{
	char buf[18];
	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return String(buf);
}

static void Ascii2MAC(String str, uint8_t *mac)
{
	sscanf(str.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
}

static void main_page(Client& client, String status = "")
{
	//index_html.print(client);
	String str((__FlashStringHelper *)index_html_flash);
	str.replace(F("%status"), status.length() > 0 ? status + '\n' : "");
	str.replace(F("%useDHCP"), IP_Config.useDHCP ? "checked": "");
	str.replace(F("%mac"), MAC2Ascii(IP_Config.mac_address));
	str.replace(F("%ip"), IP2Ascii(IP_Config.local_ip));
	str.replace(F("%subnet"), IP2Ascii(IP_Config.subnet));
	str.replace(F("%dns"), IP2Ascii(IP_Config.dns_server));
	str.replace(F("%gateway"), IP2Ascii(IP_Config.gateway));
	str.replace(F("%modbus_baudrate"), String(IP_Config.modbus_baudrate));
	client << str;
	lduino.PrintStats(client);
	client << F("</pre></body></html>");
}

static boolean index_handler(TinyWebServer& web_server)
{
	D(Serial.println("GET index.html"));
	web_server.send_error_code(200);
	web_server.send_content_type("text/html");
	web_server.end_headers();
	Client& client = web_server.get_client();
	main_page(client);
	return true;
}

static void file_uploader_handler(TinyWebServer& web_server, TinyWebPutHandler::PutAction action, char* buffer, int size)
{
	static char *boundary = NULL;
	static size_t boundary_len = 0;
	static CircularBuffer<char, 128> cb;
	static enum state { search_begin=0, search_data, in_data} st;
	static int file_size = 0;
	static char *fname;
	
	switch (action) {
	case TinyWebPutHandler::START: 
	{
		fname = web_server.get_file_from_path(web_server.get_path());
		const char *ct = web_server.get_header_value("Content-Type");
		if (!ct) break;
		boundary = strstr(ct, "boundary=");
		if (!boundary) break;
		boundary += 5;
		boundary[0] = '\r';
		boundary[1] = '\n';
		boundary[2] = '-';
		boundary[3] = '-';
		boundary_len = strlen(boundary);
		D(Serial.print("**** Upload file "));
		D(Serial.println(fname));
		free(fname);
		st = search_begin;
		file_size = 0;
		break;
	}

	case TinyWebPutHandler::WRITE:
		for (int i = 0; i < size; i++) {
			D(Serial.write(*buffer));
			cb.push(*buffer++);
			switch(st) {
			case search_begin:
				if (cb.remain() < boundary_len-2) break;
				if (cb.match(boundary+2, boundary_len-2)) {
					st = search_data;
					D(Serial.println("search_data"));
					cb.flush();
				}
				else {
					cb.pop();
				}
				break;
			case search_data:
				if (cb.remain() < 4) break;
				if (cb.match("\r\n\r\n", 4)) {
					st = in_data;
					D(Serial.println("in_data"));
					cb.flush();
				}
				else {
					cb.pop();
				}
				break;
			case in_data:
				if (cb.remain() < boundary_len) break;
				if (cb.match(boundary, boundary_len)) {
					st = search_data;
					D(Serial.println("search_data"));
					cb.flush();
				}
				else {
					if (file_size == 0) lduino.ClearProgram();
					char c = cb.pop();
					file_size++;
					lduino.LoadProgramLine(c);
				}
				break;
			};
		}
		break;

	case TinyWebPutHandler::END:
		D(Serial.println("**** END"));
		D(Serial.print("file size "));
		D(Serial.println(file_size));
		Client& client = web_server.get_client();
		String status;
		if (file_size > 0 && lduino.getProgramReady()) {
			status += F("<font color='green'>New ladder program uploaded - file size: ");
			status += file_size;
			status += F(" bytes</font>");
		}
		else {
			status += F("<font color='red'>Upload aborted</font>");
		}
		main_page(client, status);
		break;
	}
}

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

static void ParseConfig(StringParse &buf)
{
	Serial << buf << '\n';

	IP_Config.useDHCP = buf.Get(F("useDHCP")) == "true" ? true : false;
	Ascii2MAC(buf.Get(F("mac")), IP_Config.mac_address);
	Ascii2IP(buf.Get(F("ip")), IP_Config.local_ip);
	Ascii2IP(buf.Get(F("subnet")), IP_Config.subnet);
	Ascii2IP(buf.Get(F("gateway")), IP_Config.gateway);
	Ascii2IP(buf.Get(F("dns")), IP_Config.dns_server);
	IP_Config.modbus_baudrate = buf.Get(F("modbus_baudrate")).toInt();
}

static boolean config_handler(TinyWebServer& web_server) 
{
	const char* length_str = web_server.get_header_value("Content-Length");
	int length = atoi(length_str);
	uint32_t start_time = millis();
	StringParse buf;

	Client& client = web_server.get_client();

	while (buf.length() < length && client.connected() && (millis() - start_time < 30000)) {
		if (!client.available()) continue;
		buf += client.readString();
	}

	ParseConfig(buf);
	IP_Config.SaveConfig();

	web_server.send_error_code(200); 
	web_server.send_content_type("text/html");
	web_server.end_headers();

	main_page(client, F("<font color='green'>IP Config saved</font>"));

	return true;
}

static boolean reboot_handler(TinyWebServer& web_server)
{
	web_server.send_error_code(200);
	web_server.send_content_type("text/html");
	web_server.end_headers();
	Client& client = web_server.get_client();
	client << F("<html><body><pre><a href='/reboot_confirm'>Please confirm reboot</a>\n<a href='/'>Cancel</a></pre></body></html>");
	return true;
}

static boolean reboot_confirm_handler(TinyWebServer& web_server)
{
	web_server.send_error_code(200);
	web_server.send_content_type("text/html");
	web_server.end_headers();

	Client& client = web_server.get_client();
	String str((__FlashStringHelper *)reboot_confirm_html_flash);
	str.replace(F("%ip"), IP2Ascii(Ethernet.localIP()));
	client << str;

	doReset = true;
	return true;
}

void setup_PLC_Web()
{
	TinyWebPutHandler::put_handler_fn = file_uploader_handler;
	web.begin();
}

void poll_PLC_Web()
{
	web.process();
}