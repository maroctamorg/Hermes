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
#include "../networking/network-basics.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


void parse_c_str(char* buffer, int len, char* str, int str_len) {
	if(len >= str_len) {
		fprintf(stderr, "received string overflows c_str buffer, will be truncated\n");
	}
	for (int i = 0; i < len; i++) {
		if(buffer[i] == '\0') {
			len = i;
			break;	
		}
		str[i] = buffer[i];
	}	
	str[len] = '\0';
}

int main() {

	INITSOCKET();

    SOCKET socket_listen;
    if (!get_sock(&socket_listen, "3650", 0, 0, 0)) {
        fprintf(stderr, "get_sock() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("listening on PORT 3650...\n");
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

	// GATEWAY WILL RECEIVE ONLY FROM THIS ADDRESS
	printf("oncoto address is: ");
	char oncoto_address[100];
	resolve_addr("oncoto.app", oncoto_address, 100, 0, 0);
	printf("%s\n", oncoto_address);


	// SET UP SERVER SOCKET
    struct addrinfo * server_address;
    if (!get_addr(&server_address, "18.125.5.251", "3650", 0, 0)) {
        fprintf(stderr, "get_addr() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("server address is: ");
    char server_IP[100];
    char server_PORT[100];
    getnameinfo(server_address->ai_addr, server_address->ai_addrlen, server_IP, sizeof(server_IP), server_PORT, sizeof(server_PORT), NI_NUMERICHOST);
    printf("%s %s\n", server_IP, server_PORT);

    SOCKET socket_server;
	SOCKET socket_client;
	
	set_sock_timeout(socket_client, 10, 0, 0);
	set_sock_timeout(socket_server, 10, 0, 0);

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
			CLOSESOCKET(socket_client);
			continue;
		}
		
		char read[20];
        int bytes_received = recv(socket_client, &read, 19, 0);
	char received[20];
	parse_c_str(read, bytes_received, received, 20);
	// debug
	printf("Received: %s\n", received);
        if (bytes_received > 0) {
			system("./wol.sh"); 
    	
			socket_server = socket(server_address->ai_family,
	        server_address->ai_socktype, server_address->ai_protocol);
			if (!ISVALIDSOCKET(socket_server)) {
			    fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
			    return 1;
			}

			sleep(5);
			int counter = 0;	
		   	while(counter < 5) {
				if( connect(socket_server, server_address->ai_addr, server_address->ai_addrlen) == 0) break;
				counter++;
				sleep(2);
			}
    	    if (counter == 5) { 
				fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
				continue;
			}

			printf("Sending: %s/n", received);
			send(socket_server, &received, 20, 0);
			bytes_received = recv(socket_server, &received, 1, 0);
			if(bytes_received > 0) {
				send(socket_client, "s", 1, 0);
			}
		
			CLOSESOCKET(socket_server);
		}
		
	}

	freeaddrinfo(server_address);
    CLOSESOCKET(socket_listen);
	DESTROYSOCKET();
    return 0;
}

