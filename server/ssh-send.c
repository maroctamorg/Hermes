/*
 * =====================================================================================
 *
 *       Filename:  ssh-send.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/09/2022 10:00:51 PM
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
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

int main() {
	char* ssh_args[5] = {"/usr/bin/ssh", "guimaraes@macMini", "ssh -o ServerAliveInterval=300 -o ServerAliveCountMax=3 -o ExitOnForwardFailure=yes -o StrictHostKeyChecking=no -f -N -T -R 5001:localhost:32400 oncoto@oncoto.app",  NULL};

	if( execv("/usr/bin/ssh", ssh_args) == -1 ) {
		perror("execve");
		fprintf(stderr, "failed to establish ssh... [%d]\n", errno);
		return 1;
	}
	sleep(300);
	fprintf(stdout, "exiting\n");
	return 0;
}
