#ifndef TIMER_H
#define TIMER_H

/*
 * =====================================================================================
 *
 *       Filename:  timer.h
 *
 *    Description:  timer struct and routine declarations
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

typedef struct {
	int start;
	int state;
} stimer_t;

typedef struct {
	int * cpid;
	int * tunnel;
	stimer_t* timer;
} state_t;

int gettime();

int updtimer(stimer_t* timer);

void resettimer(stimer_t* timer);

#endif /* TIMER_H */
