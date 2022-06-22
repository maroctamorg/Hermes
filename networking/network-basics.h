#ifndef NETWORK_BASICS_H
#define NETWORK_BASICS_H

/*
 * =====================================================================================
 *
 *       Filename:  network-basics.h
 *
 *    Description:  declares basic network routines
 *
 *        Version:  1.0
 *        Created:  06/22/2022 09:48:29 AM
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

short get_sock(SOCKET * sock, const char * PORT, int ai_family, int ai_socktype, int ai_flags);
short get_addr(struct addrinfo ** address, const char * domain, const char * service, int ai_socktype, int ai_family);
void resolve_addr(char * domain, char * IP_addr, size_t IP_addr_s, int ai_socktype, int ai_family);
void set_sock_timeout(SOCKET sock, int s, int ms, int mu);

#endif /* NETWORK_BASICS_H */
