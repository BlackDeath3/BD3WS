#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

const char* HTTP_200_OK = "HTTP/1.1 200 OK";

const char* CONTENT_TEXT_PLAIN = "text/plain";
const char* CONTENT_TEXT_HTML = "text/html";
const char* CONTENT_TEXT_CSS = "text/css";
const char* CONTENT_IMAGE_PNG = "image/png";

const char* BD3WS_ServerName = "BD3WS";
const char* BD3WS_DefaultIP = "localhost";
const char* BD3WS_DefaultPort = "33333";
const int BD3WS_MaxLengthData = 1024;
const char* BD3WS_RootDirectory = "public/";

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

BD3WS_Server server;
BD3WS_Client client;