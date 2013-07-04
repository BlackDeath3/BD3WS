#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

void initialize();
void finalize(int exit_code);

char* server_name = "BD3 Web Server";
char* default_ip = "localhost";
char* default_port = "33333";
const int MAX_LENGTH_DATA = 1024;
struct addrinfo* server_info;
struct addrinfo hints;
struct sockaddr_in* address;
struct sockaddr_storage connection_addr;
socklen_t address_size;
char ip[INET6_ADDRSTRLEN];
int server_socket = -1;
int connection_socket = -1;
char buffer[1024] = {0};
char output[1024] = "HTTP/1.1 200 OK\nServer: BD3WS\nContent-Type: text/html\n\n<html><body>Hello world!</body></html>";

int main(int argc, char **argv)
{
	// Server start up.
	initialize();
	//

	// Return address information for the specified hostname and service.
	if(3 == argc && 0 != getaddrinfo(argv[1], argv[2], &hints, &server_info))
	{
		fprintf(stderr, "(%s | Error): Could not get specified address information!\n", server_name);
		finalize(1);
	}
	//

	// Return address information for the default hostname and service.
	else if(3 > argc && 0 != getaddrinfo(default_ip, default_port, &hints, &server_info))
	{
		fprintf(stderr, "(%s | Error): Could not get default address information!\n", server_name);
		finalize(1);
	}
	//

	// Extract address and port information.
	address = (struct sockaddr_in*)server_info->ai_addr;
	inet_ntop(server_info->ai_family, &(address->sin_addr), ip, sizeof(ip));
	//

	// Get a server socket descriptor.
	if(0 > (server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol)))
	{
		fprintf(stderr, "(%s | Error): Invalid server socket descriptor!\n", server_name);
		finalize(1);
	}
	//

	// Associate socket with port.
	if(-1 == bind(server_socket, server_info->ai_addr, server_info->ai_addrlen))
	{
		fprintf(stderr, "(%s | Error): Could not bind to socket!\n", server_name);
		finalize(1);
	}
	//

	// Finished with the server information structure.
	freeaddrinfo(server_info);
	//

	// Begin listening on the socket.
	listen(server_socket, 3);
	fprintf(stdout, "(%s | Information): Listening on %s:%hu\n", server_name, ip, ntohs(address->sin_port));
	//

	fflush(stdout);

	// Server main loop.
	while(1)
	{
		// Get a connection socket descriptor or restart the loop.
		address_size = sizeof(connection_addr);
		if(-1 == (connection_socket = accept(server_socket, (struct sockaddr *)&connection_addr, &address_size)))
		{
			fprintf(stderr, "(%s | Error): Invalid connection socket descriptor!\n", server_name);
			continue;
		}
		fprintf(stdout, "(%s | Information): Accepted connection from client.\n", server_name);
		//

		// Receive data from browser.
		if (-1 != recv(connection_socket, buffer, MAX_LENGTH_DATA, 0))
		{
			fprintf(stdout, buffer);

			// Send data to client.
			if(-1 == send(connection_socket, output, MAX_LENGTH_DATA, 0))
			{
				fprintf(stderr, "(%s | Error): Could not send data to client!\n", server_name);
				finalize(1);
			}
			//

			// Close connection socket.
			close(connection_socket);
			//
		}
		//
	}
	//

	// Server shut down.
	finalize(0);
	//
}

void initialize()
{
	fprintf(stdout, "(%s | Information): Initializing...\n", server_name);

	// Set initial server configuration.
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	//
}

void finalize(int exit_code)
{
	fprintf(stdout, "(%s | Information): Finalizing...\n", server_name);

	// Close connection socket.
	if (-1 != connection_socket)
	{
		close(connection_socket);
	}
	//

	// Close server socket.
	if (-1 != server_socket)
	{
		close(server_socket);
	}
	//

	exit(exit_code);
}

void process_request(int socket)
{

}

void send_file(int socket)
{

}