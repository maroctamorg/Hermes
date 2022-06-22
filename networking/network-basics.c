/*
 * =====================================================================================
 *
 *       Filename:  network-basics.c
 *
 *    Description:  
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

#include "network-basics.h"
#include <string.h>

short get_sock(SOCKET * sock, const char * PORT, int ai_family, int ai_socktype, int ai_flags) {
    // Configuring local address
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family	= ai_family		? ai_family		: AF_INET;
	hints.ai_socktype	= ai_socktype	? ai_socktype	: SOCK_STREAM;
	hints.ai_flags		= ai_flags		? ai_flags		: AI_PASSIVE;

    struct addrinfo * bind_address;
    getaddrinfo(0, PORT, &hints, &bind_address);

    // Creating socket
    *sock = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	int flag = ISVALIDSOCKET(*sock);
	flag = flag && ( bind(*sock, bind_address->ai_addr, bind_address->ai_addrlen) > -1 );

	freeaddrinfo(bind_address);
	return flag;
}

short get_addr(struct addrinfo ** address, const char * domain, const char * service, int ai_socktype, int ai_family) {
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype	= ai_socktype ? SOCK_STREAM : 0;
	hints.ai_family		= ai_family;

	int flag = getaddrinfo(domain, service, &hints, address);
	return !flag;
}

void resolve_addr(char * domain, char * IP_addr, size_t IP_addr_s, int ai_socktype, int ai_family) {
	struct addrinfo * address;
	get_addr(&address, domain, 0, ai_socktype, ai_family);
	getnameinfo(address->ai_addr, address->ai_addrlen,
	        IP_addr, IP_addr_s,
	        0, 0,
	        NI_NUMERICHOST);
	freeaddrinfo(address);
}

void set_sock_timeout(SOCKET sock, int s, int ms, int mu) {
	#if defined(_WIN32)
	DWORD timeout = s * 1000 + ms;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
	#else 
	struct timeval tv;
	tv.tv_sec = s;
	tv.tv_usec = ms * 1000 + mu;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	#endif
}
