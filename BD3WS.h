// External header files.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
//

// HTTP server response headers.
const char* HTTP_200_OK = "HTTP/1.1 200 OK";
const char* HTTP_404_NOTFOUND = "HTTP/1.1 404 NOT FOUND";
//

// Media content types.
const char* CONTENT_TEXT_PLAIN = "text/plain";
const char* CONTENT_TEXT_HTML = "text/html";
const char* CONTENT_TEXT_CSS = "text/css";
const char* CONTENT_IMAGE_JPEG = "image/jpeg";
const char* CONTENT_IMAGE_PNG = "image/png";
const char* CONTENT_IMAGE_XICON = "image/x-icon";
const char* CONTENT_AUDIO_WEBM = "audio/webm";
const char* CONTENT_AUDIO_OGG = "audio/ogg";
const char* CONTENT_VIDEO_WEBM = "video/webm";
const char* CONTENT_VIDEO_OGG = "video/ogg";
const char* CONTENT_ANY = "*/*";
//

#define BD3WS_MaxLengthData 2048
#define BD3WS_MaxNumberClients 16

// System constants
const char* BD3WS_ServerName = "BD3WS";
const char* BD3WS_ServerVersion = "0.1";
const char* BD3WS_DefaultIP = "localhost";
const char* BD3WS_DefaultPort = "33333";
const char* BD3WS_RootDirectory;
const char* BD3WS_PublicDirectory = "public/";
const char* BD3WS_SystemDirectory = "system/";
const char* BD3WS_LogDirectory = "system/log/";
const char* BD3WS_FileHTTP404 = "system/web/404.html";
const char* BD3WS_Log = "system/log/log.txt";
//

// HTTP status codes.
typedef enum
{
	OK = 200,
	NOTFOUND = 404,
} BD3WS_HTTPResponseState;
//

// Client connections.
typedef struct
{
	int socket;
	struct sockaddr_storage address_storage;
	socklen_t address_size;
	pthread_t thread;
	int occupied;
} BD3WS_Client;
//

// Web server.
typedef struct
{
	int socket;
	char ip[INET6_ADDRSTRLEN];
	unsigned short port;
	struct addrinfo* info;
	struct addrinfo hints;
	BD3WS_HTTPResponseState response_state;
	BD3WS_Client clients[BD3WS_MaxNumberClients];
} BD3WS_Server;
//

// Function signatures.
void initialize(int argc, char** argv);
void finalize(int exit_code);
void process_CLA(int argc, char** argv);
void setup_socket();
void extract_connection_information();
int accept_client();
void* handle_client_request(void* client);
void parse_client_request(int client, char* file_path, char* content_type);
void send_server_response(int client, const char* file_name, const char* content_type);
void build_response_header(struct stat* file_stat, const char* content_type, char* response_header);
void build_response_header_state(char* response_header);
void build_response_header_content(struct stat* file_stat, const char* content_type, char* response_header);
void log(const char* format, int error);
//

// Globals.
BD3WS_Server server;
pthread_mutex_t mutex_client_request = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_log = PTHREAD_MUTEX_INITIALIZER;
int log_handle;
//