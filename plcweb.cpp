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

#include "httpd-fsdata.h"

#if 0
#include "utility/w5100.h"
void W5100_reset(void)
{
	Serial << "W5100 reset\n";

	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100Class::writeMR(1 << 7);
	W5100Class::writeTMSR(0x55);
	W5100Class::writeRMSR(0x55);
	SPI.endTransaction();

	Serial << "Transaction done\n";

	uint8_t resetState;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	do {
		resetState = W5100Class::readMR();
		Serial << resetState << '\n';
		delay(100);
	} while ((resetState & (1 << 7)) != 0);
	SPI.endTransaction();

	delay(100);
}
#endif


extern LDuino_engine lduino;
extern IP_Config_t IP_Config;
extern bool doReset;

static boolean file_handler(TinyWebServer& web_server);
static void manage_toggle(const char * path);
static boolean getstate_handler(TinyWebServer& web_server);
static boolean setconfig_handler(TinyWebServer& web_server);
static boolean getconfig_handler(TinyWebServer& web_server);
static boolean reboot_handler(TinyWebServer& web_server);

static TinyWebServer::PathHandler handlers[] = {
	{ "/upload", TinyWebServer::POST, &TinyWebPutHandler::put_handler },
	{ "/setconfig", TinyWebServer::POST, &setconfig_handler },
	{ "/getconfig.xml", TinyWebServer::GET, &getconfig_handler },
	{ "/reboot", TinyWebServer::GET, &reboot_handler },
	{ "/getstate.xml" "*", TinyWebServer::GET, &getstate_handler },
	{ "/" "*", TinyWebServer::GET, &file_handler },
	{ NULL },
};

static const char* headers[] = {
	"Content-Length",
	"Content-Type",
	NULL
};

static TinyWebServer web = TinyWebServer(handlers, headers);

#if 0
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
#endif

static bool find_file(const char *path, const struct httpd_fsdata_segment **segment, int *len, bool *use_gzip)
{
	const struct httpd_fsdata_file *p = httpd_fsroot;
	while (p) {
		char *n = (char*)pgm_read_word(&(p->name));
		if (strcmp_P(path, n) == 0) {
			*segment = (const struct httpd_fsdata_segment *)pgm_read_word(&(p->first));
			*len = pgm_read_word(&(p->len));
			*use_gzip = pgm_read_byte(&(p->gzip));
			return true;
		}
		p = (const struct httpd_fsdata_file *)pgm_read_word(&(p->next));
	}
	return false;
}

static boolean file_handler(TinyWebServer& web_server)
{
	const char *path = web_server.get_path();
	Serial << "GET " << path << '\n';

	if (strcmp(path, "/") == 0) path = "/index.html";

	const struct httpd_fsdata_segment *segment;
	int len;
	bool use_gzip;

	if (find_file(path, &segment, &len, &use_gzip)) {
		web_server.send_error_code(200);
		web_server.send_content_type(web_server.get_mime_type_from_filename(path));
		if (use_gzip) web_server.write("Content-Encoding: gzip\n");
		web_server.end_headers(); 
	
		while (segment && len > 0) {
			char *data = (char *)pgm_read_word(&(segment->data));
			int i;
			int segment_len = min(len, httpd_fsdata_segment_len);
			
			while (segment_len > 0) {
				uint8_t buf[256];
				int i;
				for (i = 0; i < segment_len && i < sizeof(buf); i++) {
					buf[i] = pgm_read_byte(data++);
				}
				segment_len -= i;
				len -= i;
				web_server.write(buf, i);
			}
			segment = (const struct httpd_fsdata_segment *)pgm_read_word(&(segment->next));
		}
	}
	else {
		web_server.send_error_code(404);
		web_server.send_content_type("text/plain");
		web_server.end_headers();
		web_server.print("not found");
	}

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
		//main_page(client, status);
		break;
	}
}

static boolean setconfig_handler(TinyWebServer& web_server) 
{
	const char* length_str = web_server.get_header_value("Content-Length");
	int length = atoi(length_str);
	uint32_t start_time = millis();
	StringParse buf;

	Client& client = web_server.get_client();

	if (length <= 0) return true;

	while (buf.length() < length && client.connected() && (millis() - start_time < 30000)) {
		if (!client.available()) continue;
		buf += client.readString();
	}

	IP_Config.ParseConfig(buf);
	IP_Config.SaveConfig();

	return true;
}

static boolean reboot_handler(TinyWebServer& web_server)
{
	web_server.send_error_code(200);
	web_server.send_content_type("text/plain");
	web_server.end_headers();
	web_server.write("rebooting");
	doReset = true;
	return true;
}

static boolean getconfig_handler(TinyWebServer& web_server)
{
	web_server.send_error_code(200);
	web_server.send_content_type("text/xml");
	web_server.end_headers();
	web_server << IP_Config.toXML();
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

static void manage_setvalue(const char *path)
{
	char *port = strstr(path, "&set=");
	if (!port) return;
	port += 5;

	char *value = strstr(path, "&value=");
	if (!value) return;
	value += 7;

	switch (*port) {
	case 'D':
	{
		int r = atoi(port + 1);
		if (r < 0 || r > 11) return;
		r += 2;
		Serial << "setPWM " << r << " " << value <<"\n";
		lduino.setPWM(r, atoi(value));
		break;
	}
	case 'A':
	{
		int r = atoi(port + 1);
		if (r < 0 || r > 11) return;
		r += 54;
		Serial << "setAnalog " << r << " " << value << "\n";
		lduino.setAnalogInput(r, atoi(value));
		break;
	}
	}
}

static void manage_toggle(const char *path)
{	
	char *toggle = strstr(path, "&toggle=");
	if (!toggle) return;
	toggle += 8;
	
	switch (*toggle) {	
	case 'D':
	{
		int r = atoi(toggle + 1);
		if (r < 0 || r > 11) return;
		r += 2;
		Serial << "toggle " << r << "\n";
		lduino.toggleDigitalOutput(r);
		break;
	}
	case 'R':
	{
		int r = atoi(toggle + 1);
		if (r < 0 || r > 9) return;
		r += 22;
		Serial << "toggle " << r << "\n";
		lduino.toggleDigitalOutput(r);
		break;
	}
	case 'A':
	{
		int r = atoi(toggle + 1);
		if (r < 0 || r > 11) return;
		r += 54;
		Serial << "toggle " << r << "\n";
		lduino.toggleDigitalInput(r);
		break;
	}
	case 'r':
		lduino.ToggleProgramRunning();
		break;
	case 'i':
		lduino.ToggleIO_Polling();
		break;
	default:
		break;
	}

	return;
}

static boolean getstate_handler(TinyWebServer& web_server)
{
	web_server.send_error_code(200);
	web_server.send_content_type("text/xml");
	web_server.send_content_type("Connection: keep-alive");
	web_server.end_headers();

	manage_toggle(web_server.get_path());
	manage_setvalue(web_server.get_path());
	Client& stream = web_server.get_client();
	lduino.XML_State(stream);
	return true;
}
