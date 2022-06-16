/*
 * =====================================================================================
 *
 *       Filename:  gateway.c
 *
 *    Description:  listens for incoming requests, wakes server and requests RSSH port-forwarding for a particular service 
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

	// GATEWAY WILL ONLY RECEIVE ONLY FROM THIS ADDRESS
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *peer_address;
	if (getaddrinfo("oncoto.app", "https", &hints, &peer_address)) {
	    fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
	    return 1;
	}
	
	printf("oncoto address is: ");
	char oncoto_address[100];
	char oncoto_service[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
	        oncoto_address, sizeof(oncoto_address),
	        oncoto_service, sizeof(oncoto_service),
	        NI_NUMERICHOST);
	printf("%s %s\n", oncoto_address, oncoto_service);
	freeaddrinfo(peer_address);


	// SET UP SERVER SOCKET
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *server_address;
    if (getaddrinfo("18.125.5.251", "3650", &hints, &server_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("server address is: ");
    char server_IP[100];
    char server_PORT[100];
    getnameinfo(server_address->ai_addr, server_address->ai_addrlen,
            server_IP, sizeof(server_IP),
            server_PORT, sizeof(server_PORT),
            NI_NUMERICHOST);
    printf("%s %s\n", server_IP, server_PORT);

    SOCKET socket_server;
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

    while(1) {
    	
		// ACCEPT INCOMING CONNECTION ATTEMPTS
		struct sockaddr_storage client_address;
	    socklen_t client_len = sizeof(client_address);
	    socket_client = accept(socket_listen,
	            (struct sockaddr*) &client_address,
	            &client_len);
	    if (!ISVALIDSOCKET(socket_client)) {
	        fprintf(stderr, "accept() failed. (%d)\n",
	                GETSOCKETERRNO());
	        break;
	    }

        char address_buffer[100];
        getnameinfo((struct sockaddr*)&client_address,
                client_len,
                address_buffer, sizeof(address_buffer), 0, 0,
                NI_NUMERICHOST);
        printf("New connection from %s\n", address_buffer);

		if(strcmp(oncoto_address, address_buffer)) {
			printf("Reading incoming byte\n");
			char read;
        	char bytes_received = recv(socket_client, &read, 1, 0);
        	if (bytes_received > 0) {
				system("./wol.sh"); 
    		
   				 printf("Creating server socket...\n");
   				 socket_server = socket(server_address->ai_family,
   				         server_address->ai_socktype, server_address->ai_protocol);
   				 if (!ISVALIDSOCKET(socket_server)) {
   				     fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
   				     return 1;
   				 }

				sleep(15);
				 
				printf("Connecting to PC...\n");
    			if (connect(socket_server, server_address->ai_addr, server_address->ai_addrlen)) {
    		    	fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
    		    	continue;
    			}
			
				send(socket_server, "s", 1, 0);
				bytes_received = recv(socket_server, &read, 1, 0);
				if(bytes_received > 0) {
					send(socket_client, "s", 1, 0);
				}
			
				printf("Closing server socket...\n");	
				CLOSESOCKET(socket_server);
			}
		}
		
		printf("Closing client socket...\n");
		CLOSESOCKET(socket_client);
	}

	// free server_address info
	freeaddrinfo(server_address);
    
	printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);

#if defined(_WIN32)
    WSACleanup();
#endif


    printf("Finished.\n");

    return 0;
}

