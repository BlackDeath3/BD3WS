#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

const char* HTTP_200_OK = "HTTP/1.1 200 OK";
const char* HTTP_404_NOTFOUND = "HTTP/1.1 404 NOT FOUND";

const char* CONTENT_TEXT_PLAIN = "text/plain";
const char* CONTENT_TEXT_HTML = "text/html";
const char* CONTENT_TEXT_CSS = "text/css";
const char* CONTENT_IMAGE_JPEG = "image/jpeg";
const char* CONTENT_IMAGE_PNG = "image/png";
const char* CONTENT_IMAGE_XICON = "image/x-icon";
const char* CONTENT_AUDIO_OGG = "audio/ogg";
const char* CONTENT_VIDEO_WEBM = "video/webm";
const char* CONTENT_ANY = "*/*";

const char* BD3WS_ServerName = "BD3WS";
const char* BD3WS_DefaultIP = "localhost";
const char* BD3WS_DefaultPort = "33333";
const char* BD3WS_RootDirectory = "public/";
const short BD3WS_MaxLengthData = 1024;

const char* BD3WS_FileHTTP404 = "system_web_pages/404.html";

typedef struct
{
	int socket;
	struct sockaddr_storage address_storage;
	socklen_t address_size;
} BD3WS_Client;

typedef struct
{
	int socket;
	char ip[INET6_ADDRSTRLEN];
	unsigned short port;
	struct addrinfo* info;
	struct addrinfo hints;
} BD3WS_Server;

typedef enum
{
	OK = 200,
	NOTFOUND = 404,
} BD3WS_HTTPResponseState;

void initialize(int argc, char** argv);
void finalize(int exit_code);
void process_CLA(int argc, char** argv);
void setup_socket();
void extract_connection_information();
int accept_client();
void process_client_request(char* file_path, char* content_type);
void send_file(const char* file_name, const char* content_type);
void prepare_response_header(struct stat* file_stat, const char* content_type, char* response_header);
void prepare_response_header_state(char* response_header);
void prepare_response_header_content(struct stat* file_stat, const char* content_type, char* response_header);

BD3WS_Server server;
BD3WS_Client client;
BD3WS_HTTPResponseState response_state;