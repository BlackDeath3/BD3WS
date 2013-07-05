#include "BD3WS.h"

int main(int argc, char **argv)
{
	char file_path[BD3WS_MaxLengthData];
	char content_type[BD3WS_MaxLengthData];

	memset(file_path, 0, sizeof(file_path));
	memset(content_type, 0, sizeof(content_type));

	// Server start up.
	initialize(argc, argv);
	//

	// Begin listening on the socket.
	listen(server.socket, 3);
	fprintf(stdout, "(%s | Information): Listening on %s:%hu\n", BD3WS_ServerName, server.ip, server.port);
	//

	// Flush output buffers before accepting any connections.
	fflush(stdout);
	fflush(stderr);
	//

	// Server main loop.
	while(1)
	{
		// Accept connection request from client.
		if(1 != accept_client())
		{
			continue;
		}
		//

		// Receive file data request from client.
		process_client_request(&file_path, &content_type);
		//

		// Send requested file data to client.
		send_file(&file_path, &content_type);

		// Close connection socket.
		close(client.socket);
		//
	}
	//

	// Server shut down.
	finalize(0);
	//
}

void initialize(int argc, char** argv)
{
	fprintf(stdout, "(%s | Information): Initializing...\n", BD3WS_ServerName);

	// Set initial server configuration.
	memset(&(server.hints), 0, sizeof(server.hints));
	server.hints.ai_family = AF_UNSPEC;
	server.hints.ai_socktype = SOCK_STREAM;
	server.hints.ai_flags = AI_PASSIVE;
	//

	// Process command-line arguments.
	process_CLA(argc, argv);
	//

	// Extract server connection information.
	extract_connection_information();

	// Setup server socket and bind it to a port.
	setup_socket();
	//

	// Finished with the server information structure.
	freeaddrinfo(server.info);
	//
}

void finalize(int exit_code)
{
	fprintf(stdout, "(%s | Information): Finalizing...\n", BD3WS_ServerName);

	// Close connection socket.
	if (-1 != client.socket)
	{
		close(client.socket);
	}
	//

	// Close server socket.
	if (-1 != server.socket)
	{
		close(server.socket);
	}
	//

	exit(exit_code);
}

void process_CLA(int argc, char** argv)
{
	// Return address information for the specified hostname and service.
	if(3 == argc && 0 != getaddrinfo(argv[1], argv[2], &(server.hints), &(server.info)))
	{
		fprintf(stderr, "(%s | Error): Could not get specified address information!\n", BD3WS_ServerName);
		finalize(1);
	}
	//

	// Return address information for the default hostname and service.
	else if(3 > argc && 0 != getaddrinfo(BD3WS_DefaultIP, BD3WS_DefaultPort, &(server.hints), &(server.info)))
	{
		fprintf(stderr, "(%s | Error): Could not get default address information!\n", BD3WS_ServerName);
		finalize(1);
	}
	//
}

void extract_connection_information()
{
	struct sockaddr_in* address;

	// Extract address and port information and print it out.
	address = (struct sockaddr_in*)server.info->ai_addr;
	inet_ntop(server.info->ai_family, &(address->sin_addr), server.ip, sizeof(server.ip));
	server.port = ntohs(address->sin_port);
	//
}

void setup_socket()
{
	// Get a server socket descriptor.
	if(0 > (server.socket = socket(server.info->ai_family, server.info->ai_socktype, server.info->ai_protocol)))
	{
		fprintf(stderr, "(%s | Error): Invalid server socket descriptor!\n", BD3WS_ServerName);
		finalize(1);
	}
	//

	// Associate socket with port.
	if(-1 == bind(server.socket, server.info->ai_addr, server.info->ai_addrlen))
	{
		fprintf(stderr, "(%s | Error): Could not bind to socket!\n", BD3WS_ServerName);
		finalize(1);
	}
	//
}

int accept_client()
{
	// Get a connection socket descriptor or restart the loop.
	client.address_size = sizeof(client.address_storage);
	if(-1 == (client.socket = accept(server.socket, (struct sockaddr *)&(client.address_storage), &(client.address_size))))
	{
		fprintf(stderr, "(%s | Error): Invalid connection socket descriptor!\n", BD3WS_ServerName);
		return 0;
	}
	fprintf(stdout, "(%s | Information): Accepted connection request from client.\n", BD3WS_ServerName);
	//

	return 1;
}

void process_client_request(char* file_path, char* content_type)
{
	char request[BD3WS_MaxLengthData];
	char line_file[BD3WS_MaxLengthData];
	char line_type[BD3WS_MaxLengthData];
	char* token = NULL;

	memset(request, 0, sizeof(request));

	// Attempt to receive a client request.
	if(-1 != recv(client.socket, request, BD3WS_MaxLengthData, 0))
	{
		fprintf(stdout, "\n\t\t\tClient Request");
		fprintf(stdout, "\n===========================================================\n");
		fprintf(stdout, "%s\n", request);
		fprintf(stdout, "\n===========================================================\n");
		fflush(stdout);

		token = strtok(request, "\n");

		// Isolate string containing file path.
		while(NULL != token)
		{
			// Isolate line containing file path.
			if (NULL != strstr(token, "GET"))
			{
				strcpy(line_file, token);
			}
			//

			// Isolate line containing content type.
			else if (NULL != strstr(token, "Accept:"))
			{
				strcpy(line_type, token);
			}
			//

			token = strtok(NULL, "\n");
		}
		//

		// Grab the file path piece of the request.
		token = strtok(line_file, " ");
		token = strtok(NULL, " ");
		//

		// Copy requested file path for output parameter.
		strcpy(file_path, token);
		//

		// Grab the content type piece of the request.
		token = strtok(line_type, " ");
		token = strtok(NULL, " ");
		strcpy(line_type, token);
		token = strtok(line_type, ",");
		//

		// Copy requested content type for output parameter.
		strcpy(content_type, token);
		//
	}
	//
}

void send_file(const char* file_name, const char* content_type)
{
	FILE* file_handle = NULL;
	struct stat file_stat;
	char file_path[BD3WS_MaxLengthData];
	char buffer[BD3WS_MaxLengthData];
	char response_header[BD3WS_MaxLengthData];
	int bytes_read = 0;

	memset(buffer, 0, sizeof(buffer));
	memset(response_header, 0, sizeof(response_header));

	// Create full file path.
	strcpy(file_path, BD3WS_RootDirectory);
	strcat(file_path, file_name);
	//

	// Open file.
	file_handle = fopen(file_path, "r");
	//

	// Check file existence to create proper response header.
	if (NULL == file_handle)
	{
		fprintf(stderr, "(%s | Error): Could not serve file: \"%s\"!\n", BD3WS_ServerName, file_path);
		response_state = 404;
	}
	else
	{
		response_state = 200;
	}
	//

	// If HTTP 404 occurs, serve error 404 page.
	if (404 == response_state)
	{
		strcpy(file_path, BD3WS_FileHTTP404);
		file_handle = fopen(file_path, "r");
	}
	//

	stat(file_path, &file_stat);

	prepare_response_header(&file_stat, content_type, response_header);

	// Send response header to client.
	if (-1 == send(client.socket, response_header, strlen(response_header), 0))
	{
		fprintf(stderr, "(%s | Error): Could not send response header to client!\n", BD3WS_ServerName);
		return;
	}
	//

	// Read file and send it to client.
	while(!feof(file_handle))
	{
		// Read requested file data.
		bytes_read = fread(buffer, 1, BD3WS_MaxLengthData, file_handle);
		//

		// If there are no more bytes to read, quit serving file.
		if (0 >= bytes_read)
		{
			break;
		}
		//

		// Send requested file data to client.
		if(-1 == send(client.socket, buffer, BD3WS_MaxLengthData, 0))
		{
			fprintf(stderr, "(%s | Error): Could not send file data to client!\n", BD3WS_ServerName);
			break;
		}
		//

		memset(buffer, 0, sizeof(buffer));
	}
	//

	fclose(file_handle);

	fprintf(stdout, "(%s | Information): File sent successfully!\n", BD3WS_ServerName);
}

void prepare_response_header(struct stat* file_stat, const char* content_type, char* response_header)
{
	prepare_response_header_state(response_header);

	strcat(response_header, "\n");
	strcat(response_header, "Server: ");
	strcat(response_header, BD3WS_ServerName);
	strcat(response_header, "\n");

	prepare_response_header_content(file_stat, content_type, response_header);

	strcat(response_header, "\n\n");

	fprintf(stdout, "\tResponse Header:\n%s", response_header);
	fprintf(stdout, "\n===========================================================\n");
	fflush(stdout);
}

void prepare_response_header_state(char* response_header)
{
	// Determine HTTP response state of requested file.
	if (200 == response_state)
	{
		strcat(response_header, HTTP_200_OK);
	}
	else if (404 == response_state)
	{
		strcat(response_header, HTTP_404_NOTFOUND);
	}
	//
}

void prepare_response_header_content(struct stat* file_stat, const char* content_type, char* response_header)
{
	char file_size[16];

	strcat(response_header, "Content-Type: ");

	// Content-Type: text/plain
	if (0 == strcmp(CONTENT_TEXT_PLAIN, content_type))
	{
		strcat(response_header, CONTENT_TEXT_PLAIN);
		strcat(response_header, ";charset=UTF-8");
	}
	//

	// Content-Type: text/html
	else if (0 == strcmp(CONTENT_TEXT_HTML, content_type))
	{
		strcat(response_header, CONTENT_TEXT_HTML);
		strcat(response_header, ";charset=UTF-8");
	}
	//

	// Content-Type: text/css
	else if (0 == strcmp(CONTENT_TEXT_CSS, content_type))
	{
		strcat(response_header, CONTENT_TEXT_CSS);
	}
	//

	// Content-Type: image/png
	else if (0 == strcmp(CONTENT_IMAGE_PNG, content_type))
	{
		strcat(response_header, CONTENT_IMAGE_PNG);
	}
	//

	// Content-Type: image/jpeg
	else if (0 == strcmp(CONTENT_IMAGE_JPEG, content_type))
	{
		strcat(response_header, CONTENT_IMAGE_JPEG);
	}
	//

	// Content-Type: image/x-icon
	else if (0 == strcmp(CONTENT_IMAGE_XICON, content_type))
	{
		strcat(response_header, CONTENT_IMAGE_XICON);
	}
	//

	// Content-Tye: audio/ogg
	else if (0 == strcmp(CONTENT_AUDIO_OGG, content_type))
	{
		strcat(response_header, CONTENT_AUDIO_OGG);
	}
	//

	// Content-Type: video/webm
	else if (0 == strcmp(CONTENT_VIDEO_WEBM, content_type))
	{
		strcat(response_header, CONTENT_VIDEO_WEBM);
	}
	//

	// Content-Type: */*
	else
	{
		strcat(response_header, CONTENT_ANY);
	}

	sprintf(file_size, "%d", file_stat->st_size);
	strcat(response_header, "\nContent-Length: ");
	strcat(response_header, file_size);
}