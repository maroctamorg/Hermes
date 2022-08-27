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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#include "../utils/timer.h"
#include "../networking/network-basics.h"

typedef struct {
	int status;
	int * cpid;
	stimer_t* timer;
} state_t;


void * timer_f(void * stt) {
	state_t * state = (state_t *) stt;
	
	resettimer(state->timer);
	while(1) {
		
		if( waitpid(*(state->cpid), &(state->status), WNOHANG) != 0 ) { 
			sleep(1800);
			continue;
		}

		printf("timer: %d\n", updtimer(state->timer));
		if(state->timer->state > 1800) {
			printf( "killing ssh tunnel, pid %d\n", *(state->cpid) );
			kill( *(state->cpid), SIGKILL );
			resettimer(state->timer);
		}

		sleep(1800);
	}
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

    printf("Waiting for connections...\n");

	// SERVER WILL RECEIVE ONLY FROM THIS ADDRESS
	char * gateway_address = "18.125.5.100";
	printf("Remote address is: %s\n", gateway_address);
    
	SOCKET socket_client;
	set_sock_timeout(socket_client, 10, 0, 0);	
   
	char* ssh_args[6] = {"/usr/bin/ssh", "-o ServerAliveInterval=300", "-NR", "5000:localhost:5000", "oncoto@oncoto.app", NULL};

	stimer_t timer;
	resettimer(&timer);
	int cpid = -1;
	
	state_t state;
	state.cpid = &cpid;
	state.timer = &timer;

	pthread_t timer_thr;
	int timer_thrd = pthread_create(&timer_thr, NULL, timer_f, (void *) &state);

	while(1) {

		// ACCEPT INCOMING CONNECTION ATTEMPTS
		struct sockaddr_storage client_address;
		socklen_t client_len = sizeof(client_address);
		socket_client = accept(socket_listen,
	            (struct sockaddr*) &client_address,
	            &client_len);
		if (!ISVALIDSOCKET(socket_client)) {
			fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
	        break;
		}
		
		char address_buffer[100];
		getnameinfo((struct sockaddr*)&client_address,
		        client_len,
		        address_buffer, sizeof(address_buffer), 0, 0,
		        NI_NUMERICHOST);
		printf("New connection from %s\n", address_buffer);
		
		if(strcmp(gateway_address, address_buffer)) {
			CLOSESOCKET(socket_client);
			continue;
		}
		
		char read;
		int bytes_received = recv(socket_client, &read, 1, 0);
		
		if (bytes_received > 0) {
			resettimer(&timer);
			if(waitpid(cpid, &(state.status), WNOHANG) == 0) {
				send(socket_client, "s", 1, 0);
				CLOSESOCKET(socket_client);	
				continue;
			}
			cpid = fork();
			if(cpid == 0) {
				printf("/usr/bin/ssh -o \"ServerAliveInterval=300\" -fNR 5000:localhost:5000 oncoto@oncoto.app\n");
				if( execv("/usr/bin/ssh", ssh_args) == -1) {
					fprintf(stderr, "failed to establish ssh tunnel in child process... [%d]\n", errno);
					return 1;
				}
				return 0;
			}
		}
		
		CLOSESOCKET(socket_client);
	}

	if(waitpid(cpid, &(state.status), WNOHANG) == 0) {
		kill(cpid, SIGKILL);
	}

    CLOSESOCKET(socket_listen);
	DESTROYSOCKET();

    return 0;
}
