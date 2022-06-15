/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  listens for incoming requests, establishes SSH port-forwarding for a particular service 
 *
 *        Version:  1.0
 *        Created:  06/14/2022 05:07:23 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  maroctamorg (mg), marcelogn.dev@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include "../networking/network-include.h"
#include "../networking/portable-macros.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

typedef struct {
	int state;
} stimer_t;

int gettime() {
	return clock() / CLOCKS_PER_SEC;
}

int updtimer(stimer_t* timer) {
	int diff = gettime() - timer->state;
	timer->state = timer->state + diff;

	return diff;
}

void resettimer(stimer_t* timer) {
	timer->state = 0;
}

int main() {

#if defined(_WIN32)
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) {
        fprintf(stderr, "Failed to initialize.\n");
        return 1;
    }
#endif

    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, "3650", &hints, &bind_address);


    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
            bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

	printf("Binding socket to local address...\n");
    if (bind(socket_listen,
                bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bind_address);


    printf("Listening on PORT 3650...\n");
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Waiting for connections...\n");

	// GATEWAY WILL RECEIVE ONLY FROM THIS ADDRESS
	printf("Configuring remote address...\n");
	struct addrinfo cl_hints;
	memset(&cl_hints, 0, sizeof(cl_hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *peer_address;
	if (getaddrinfo("18.125.5.100", 0, &cl_hints, &peer_address)) {
	    fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
	    return 1;
	}
	
	printf("Remote address is: ");
	char gateway_address[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
	        gateway_address, sizeof(gateway_address),
	        0, 0,
	        NI_NUMERICHOST);
	printf("%s\n", gateway_address);
    
	// SET UP CLIENT SOCKET
	SOCKET socket_client;
	
		// SET TIMEOUT
	#if defined(_WIN32)
	DWORD timeout = timeout_in_seconds * 1000;
	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
	#else 
	int timeout_in_seconds = 10;
	struct timeval tv;
	tv.tv_sec = timeout_in_seconds;
	tv.tv_usec = 0;
	setsockopt(socket_client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	#endif
   
	char* ssh_args[5] = {"-o", "\"ServerAliveInterval 300\"", "-fNR", "5000:localhost:5000", "oncoto@oncoto.app"};

	stimer_t timer;
	resettimer(&timer);
	int cpid = -1;
	int tunnel = 1;
	while(1) {
		if(tunnel) updtimer(&timer);
		if(timer.state > 600) {
			kill(cpid, SIGKILL);
			tunnel = 1;
		}

		// ACCEPT INCOMING CONNECTION ATTEMPTS
		struct sockaddr_storage client_address;
	    socklen_t client_len = sizeof(client_address);
	    socket_client = accept(socket_listen,
	            (struct sockaddr*) &client_address,
	            &client_len);
	    if (!ISVALIDSOCKET(socket_client)) {
	        fprintf(stderr, "accept() failed. (%d)\n",
	                GETSOCKETERRNO());
	        return 1;
	    }

        char address_buffer[100];
        getnameinfo((struct sockaddr*)&client_address,
                client_len,
                address_buffer, sizeof(address_buffer), 0, 0,
                NI_NUMERICHOST);
        printf("New connection from %s\n", address_buffer);

		if(strcmp(gateway_address, address_buffer)) {
			printf("Reading incoming byte\n");
			char read;
        	char bytes_received = recv(socket_client, &read, 1, 0);
        	
			if (bytes_received > 0) {
				if(tunnel) {
					resettimer(&timer);
				} else {
					cpid = fork();
					tunnel = 0;
					resettimer(&timer);
					printf("establishing tunnel...\n");
					if(cpid == 0) {
						execv("/usr/bin/ssh", ssh_args);
					}
				}
				send(socket_client, "s", 1, 0);
			}
		}
		
		printf("Closing client socket...\n");
		CLOSESOCKET(socket_client);
	}

	if(tunnel) {
		kill(cpid, SIGKILL);
		tunnel = 1;
	}

    printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);

#if defined(_WIN32)
    WSACleanup();
#endif


    printf("Finished.\n");

    return 0;
}

