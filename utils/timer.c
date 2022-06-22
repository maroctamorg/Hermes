/*
 * =====================================================================================
 *
 *       Filename:  timer.c
 *
 *    Description:  timer routines
 *
 *        Version:  1.0
 *        Created:  06/22/2022 07:00:00 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  maroctamorg (mg), marcelogn.dev@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <sys/time.h>
#include <time.h>
#include "timer.h"

int gettime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec;
}

int updtimer(stimer_t* timer) {
	timer->state = gettime() - timer->start;

	return timer->state;
}

void resettimer(stimer_t* timer) {
	timer->start = gettime();
	timer->state = 0;
}

