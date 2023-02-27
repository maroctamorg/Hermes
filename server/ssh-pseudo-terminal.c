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
	int cpid = fork();
	if(cpid == 0) {
		fprintf(stdout, "starting ssh\n");
		FILE *ssh = popen("/usr/bin/ssh -f guimaraes@macMini", "w");
		if(!ssh) {
			fprintf(stderr, "failed to establish ssh in child process... [%d]\n", errno);
			return 1;
		}
		fprintf(ssh, "ssh -tt -C -NR 5001:localhost:32400 oncoto@oncoto.app");
		sleep(300);
		fprintf(stdout, "closing ssh\n");
		pclose(ssh);
		return 0;
	}

	waitpid(cpid, NULL, 0);
	return 0;
}
