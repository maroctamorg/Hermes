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

#define INTERVAL 3600

typedef struct {
	int status;
	int * cpid;
	stimer_t* timer;
} state_t;

int open_service(state_t * state, int lport, int rport) {
	if(waitpid(*(state->cpid), &(state->status), WNOHANG) == 0) {
		return 1;
	}

	*(state->cpid) = fork();
	if(*(state->cpid) != 0) {
		return 1;
	}

	char pmap[25];
	snprintf(pmap, 25, "%d:localhost:%d", rport, lport);

	char* ssh_args[6] = {"/usr/bin/ssh", "-o ServerAliveInterval=300", "-NR", pmap, "oncoto@oncoto.app", NULL};
	printf("/usr/bin/ssh -o \"ServerAliveInterval=300\" -fNR %s oncoto@oncoto.app\n", pmap);
	if( execv("/usr/bin/ssh", ssh_args) == -1) {
		fprintf(stderr, "failed to establish ssh tunnel in child process... [%d]\n", errno);
		return 1;
	}
	return 0;
}

void close_plex(int signum) {
	if(signum == 0) {
		sleep(3*INTERVAL);
	}

	printf("\t(SIGINT) [killing plex tunnel on macMini]\n");
	//char* sshkill_args[3] = {"/home/magneto/services/hermes/ssh-send", "kill `(ps aux | grep \"ssh -C -NR 5001:localhost:32400 oncoto@oncoto.app\"| grep -v grep | awk '{print $2}')`", NULL};
	char* sshkill_args[4] = {"/usr/bin/ssh", "guimaraes@macMini", "kill `(ps aux | grep \"ssh -C -NR 5001:localhost:32400 oncoto@oncoto.app\"| grep -v grep | awk '{print $2}')`", NULL};
	if( execv("/usr/bin/ssh", sshkill_args) == -1) {
		fprintf(stderr, "failed to kill plex ssh tunnel on macMini... [%d]\n", errno);
	}
}

int open_plex() {
	int pid = fork();
	if(pid != 0) {
		return 0;
	}

	fprintf(stdout, "establishing plex tunnel on macMini...\n");
	char* sshsend_args[2] = {"/home/magneto/services/hermes/ssh-send", NULL};
	if( execv("/home/magneto/services/hermes/ssh-send", sshsend_args) == -1) {
		fprintf(stderr, "failed to establish plex ssh tunnel on macMini... [%d]\n", errno);
		return 1;
	}
	return 0;
}


void * timer_f(void * stt) {
	state_t * state = (state_t *) stt;
	
	resettimer(state->timer);
	while(1) {
		
		if( waitpid(*(state->cpid), &(state->status), WNOHANG) != 0 ) { 
			sleep(INTERVAL/2);
			continue;
		}

		printf("timer: %d\n", updtimer(state->timer));
		if(state->timer->state > INTERVAL) {
			printf( "terminating ssh, pid %d\n", *(state->cpid) );
			kill( *(state->cpid), SIGTERM );
			resettimer(state->timer);
		}

		sleep(INTERVAL/2);
	}
}

void parse_c_str(char* buffer, int len, char* str, int str_len) {
        if(len >= str_len) {
                fprintf(stderr, "received string overflows c_str buffer, will be truncated\n");
        }
        for (int i = 0; i < len; i++) {
                // debug
                printf("parsing char: %c\n", buffer[i]);
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

	// SERVER WILL RECEIVE ONLY FROM THIS ADDRESS
	char * gateway_address = "18.125.5.100";
	printf("Remote address is: %s\n", gateway_address);
    
	SOCKET socket_client;
	set_sock_timeout(socket_client, 10, 0, 0);	
   

	stimer_t atimer;
	resettimer(&atimer);
	int acpid = -1;
	
	state_t astate;
	astate.cpid = &acpid;
	astate.timer = &atimer;

	pthread_t atimer_thr;
	int atimer_thrd = pthread_create(&atimer_thr, NULL, timer_f, (void *) &astate);


	struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = close_plex;
    sigaction(SIGINT, &action, NULL);

	while(1) {
    	printf("Waiting for connections...\n");
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
		
		char read[20];
		int bytes_received = recv(socket_client, &read, 20-1, 0);
		printf("received: %s\n", read);
		
    char plex[20] = "plex";
    char alexandria[20] = "alexandria";
    char mstream[20] = "mstream";
    char audiobookshelf[20] = "audiobookshelf";

		int tcpid = -1;
		if (bytes_received > 0) {
			send(socket_client, &read, 1, 0);
			if(strcmp(read, plex) == 0) {
			  int status = 0;
			  int pid = -1;
			  pid = fork();
			  if(pid == 0) {
			  	close_plex(1);
			  	return EXIT_SUCCESS;
			  }
			  waitpid(pid, &status, 0);
			  open_plex();
      }
			if(strcmp(read, alexandria) == 0) {
			  resettimer(&atimer);
			  open_service(&astate, 5000, 5000);
      }
			if(strcmp(read, mstream) == 0) {
				resettimer(&atimer);
				open_service(&astate, 3000, 6300);
      }
      if(strcmp(read, audiobookshelf) == 0) {
        // debug
        printf("received '%s': opening service\n", audiobookshelf);
				resettimer(&atimer);
				open_service(&astate, 5001, 5004);
			}
		}
		CLOSESOCKET(socket_client);
	}
	
	if(waitpid(acpid, &(astate.status), WNOHANG) == 0) {
		kill(acpid, SIGKILL);
	}

    CLOSESOCKET(socket_listen);
	DESTROYSOCKET();

    return 0;
}
