// 
// 
// 

#include <Ethernet.h>
#include <Flash.h>

#include "TinyWebServer.h"
#include "plcweb.h"
#include "CircularBuffer.h"
#include "lduino_engine.h"

extern LDuino_engine lduino;
static boolean index_handler(TinyWebServer& web_server);

FLASH_STRING(index_html,
	"<html><body>"
	"<h2>LDuino PLC</h2>"
	"(c) 2016 Frederic Rible (frible@teaser.fr)"
	"<hr>"
	"<form action = '/'"
	"enctype = 'multipart/form-data' method = 'post'>"
	"<br>"
	"Please specify a LDmicro XINT file : <br>"
	"<input type = 'file' name = 'datafile' size = '40'>"
	"<div>"
	"<input type = 'submit' value = 'Send'>"
	"</div>"
	"</form>"
	"<a href='/'>Refresh</a>"
	);

static TinyWebServer::PathHandler handlers[] = {
	{ "/" "*", TinyWebServer::GET, &index_handler },
	{ "/" "*", TinyWebServer::POST, &TinyWebPutHandler::put_handler },
	{ NULL },
};

static const char* headers[] = {
	"Content-Length",
	"Content-Type",
	NULL
};

static TinyWebServer web = TinyWebServer(handlers, headers);

static void main_page(Client& client)
{
	index_html.print(client);
	client << "<br>PLC time: " << lduino.GetTime() << " ms";
	client << "<br>PLC processing time: " << lduino.GetProcessingTime() << " us";
	client << "</body></html>";
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
					lduino.ClearProgram();
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
		main_page(client);
		client << "<h2>New ladder program uploaded - size: " << file_size << " bytes</h2>";
		break;
	}
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