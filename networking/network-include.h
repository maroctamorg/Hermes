/*
 * =====================================================================================
 *
 *       Filename:  network-include.h
 *
 *    Description:  compiles required header files for UNIX/WIN sockets 
 *
 *        Version:  1.0
 *        Created:  05/17/2022 05:12:28 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  maroctamorg (mg), marcelogn.dev@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#if defined(_WIN32)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#if !defined(IPV6_V6ONLY)
#define IPV6_V6ONLY 27
#endif

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#endif
