/*
 * =====================================================================================
 *
 *       Filename:  portable-macros.h
 *
 *    Description:  This header file defines C socket macros for Unix-Win portability 
 *
 *        Version:  1.0
 *        Created:  05/17/2022 04:57:01 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  maroctamorg (mg), marcelogn.dev@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#if defined(_WIN32) // WIN
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())
#define INITSOCKET() {\
	WSADATA d;\
	if (WSAStartup(MAKEWORD(2, 2), &d)) {\
		fprintf(stderr, "Failed to initialize WINSOCKET.\n");\
		return 1;\
	}\
}
#define DESTROYSOCKET() WSACleanup()


#else // UNIX
#define SOCKET int
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define GETSOCKETERRNO() (errno)
#define INITSOCKET() 
#define DESTROYSOCKET()

#endif
