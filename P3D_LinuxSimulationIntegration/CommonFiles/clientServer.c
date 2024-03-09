// Copyright (c) 2010-2022 Lockheed Martin Corporation. All rights reserved.
// Use of this file is bound by the PREPAR3D® SOFTWARE DEVELOPER KIT END USER LICENSE AGREEMENT
    
// ClientServer.c
// Description: Opens either a host or client connection.

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef LINUX
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
typedef int SOCKET;
#define closesocket close
#else
#define INET_ADDRSTRLEN 16
#define _WINSOCK_DEPRECATED_NO_WARNINGS
typedef int socklen_t;
#include    <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#endif
#include <fcntl.h>
#include "clientServer.h"

static const int           trace = FALSE;

typedef struct
{
    // network connection-related attributes
    SOCKET              socketFd;
    struct sockaddr_in  hostAddr;
    socklen_t           hostAddrLength;
    unsigned int        msgsSentQty;
    unsigned int        msgsRcvdQty;
    int                 inServerMode;
    int                 portnum;
} csRealInfo_t;


// Rather simple - The server passes NULL for
// host name and the portent that it will be listening on. The client
// passes the name of the server machine and the same port number.
csInfo_t* csOpen(char* hostname, int portnum)
{
    int status = -1;
    csRealInfo_t* csInfo;


    if (trace)
    {
        printf("csInit: entering\n");
        printf("csInit: using network interface to %s on "
            "port %d (htons=%d)\n", hostname, portnum, htons(portnum));
    }

    if ((csInfo = (csRealInfo_t*)calloc(1, sizeof(*csInfo))) == NULL)
    {
        printf("Failed to allocate memory for csInfo: %s\n", strerror(errno));
    }

    // Open receive side of socket. If given a host name, that will be who
    // all messages will go to.  Otherwise, we will just send back to whichever
    // client sent us the last packet.
    else if ((csInfo->socketFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("Failed to open UDP socket: %s\n", strerror(errno));
    }
    else
    {
        csInfo->msgsSentQty = 0;
        csInfo->msgsRcvdQty = 0;
        csInfo->portnum = portnum;
        csInfo->hostAddr.sin_family = AF_INET;
        csInfo->hostAddr.sin_port = htons(portnum);
        csInfo->hostAddrLength = sizeof(csInfo->hostAddr);

        if (hostname == NULL) // server setup
        {
            csInfo->inServerMode = TRUE;

            // bind to port on any supported DGRAM interface
            csInfo->hostAddr.sin_addr.s_addr = htonl(INADDR_ANY);
            if (bind(csInfo->socketFd,
                (struct sockaddr *)&csInfo->hostAddr,
                sizeof(csInfo->hostAddr)) < 0)
            {
                printf("Failed to bind UDP socket: %s\n", strerror(errno));
            }
            else
            {
                status = 0;     // success!
            }
        }

        else // client setup
        {
            csInfo->inServerMode = FALSE;

            // For the client, we don't need to do a bind() -- the first
            // sendto() will implicitly bind to any available port.  Just
            // setup the hostAddr.
            struct hostent* host;
            if ((host = gethostbyname(hostname)) == NULL)
            {
                printf("Can't find address for host '%s'\n", hostname);
            }

            else
            {
                csInfo->hostAddr.sin_addr.s_addr =
                    *(unsigned long*)(host->h_addr);
                status = 0;     // success!

                if (trace)
                {
                    printf("host: will send all datagrams to:\n");
                    printf("  hostAddr.sin_family = %d\n",
                        csInfo->hostAddr.sin_family);
                    printf("  hostAddr.sin_port = %d\n",
                        csInfo->hostAddr.sin_port);
                    printf("  hostAddr.sin_addr = %s\n",
                        inet_ntoa(csInfo->hostAddr.sin_addr));
                }
            }
        }
    }

    if (status != 0)
    {
        csClose((csInfo_t*)csInfo);
        csInfo = NULL;
    }

    if (trace)
    {
        printf("port=%d\n", portnum);
        printf("leaving\n");
    }

    return ((csInfo_t*)csInfo);
}


/*
 * Sets the socket to blocking (default) or non-blocking.  For non-blocking,
 * csRead() returns -1 if there is no UDP packet available.
 */
int csSetNonBlockingState(csInfo_t* p_csInfo, int isNonBlocking)
{
    int status = -1;

    csRealInfo_t* csInfo = (csRealInfo_t*)p_csInfo;
    if (csInfo == NULL)
    {
        errno = EINVAL;
    }

    else
    {
        if (trace)
        {
            printf("setting socket to non-blocking\n");
        }

        if (isNonBlocking)
        {
#ifdef LINUX
            // set the socket to non-blocking
            if (fcntl(csInfo->socketFd, F_SETFL,
                fcntl(csInfo->socketFd, F_GETFL, 0)|O_NONBLOCK) < 0)
            {
                printf("Couldn't set non-blocking: %s\n", strerror(errno));
            }
#else
			int iMode = 1;
			if (ioctlsocket(csInfo->socketFd, FIONBIO, &iMode) != NO_ERROR)
			{
				printf("Couldn't set non-blocking: %s\n", strerror(errno));
			}
#endif
            else
            {
                status = 0;
            }
        }
        else
        {
#ifdef LINUX 
			// set the socket to blocking
            if (fcntl(csInfo->socketFd, F_SETFL,
                fcntl(csInfo->socketFd, F_GETFL, 0) & ~O_NONBLOCK) < 0)
            {
                printf("Couldn't set blocking: %s\n", strerror(errno));
            }
#else
			int iMode = 0;
			if (ioctlsocket(csInfo->socketFd, FIONBIO, &iMode) != NO_ERROR)
			{
				printf("Couldn't set non-blocking: %s\n", strerror(errno));
			}
#endif
            else
            {
                status = 0;
            }
        }
    }

    return (status);
}


/*
 * Returns the number of bytes currently queued for reading.
 */
int csGetQueuedQty(csInfo_t* p_csInfo)
{
   int queuedQty;


    csRealInfo_t* csInfo = (csRealInfo_t*)p_csInfo;
    if (csInfo == NULL)
    {
        queuedQty = -1;
        errno = EINVAL;
    }
#ifdef LINUX
    else if (ioctl(csInfo->socketFd, FIONREAD, &queuedQty) == -1)
#else
    else if (ioctlsocket(csInfo->socketFd, FIONREAD, &queuedQty) == -1)
#endif
    {
	printf("unexpected error from ioctl() - errno=%d '%s'\n", errno, strerror(errno));
        queuedQty = -1;
    }

    return (queuedQty);

}


int csRead(csInfo_t* p_csInfo, void* inbuf, ssize_t requestedQty)
{
    ssize_t            rcvdQty;
    struct sockaddr_in tmpAddr;
    socklen_t          tmpAddrLength = sizeof(tmpAddr);


    csRealInfo_t* csInfo = (csRealInfo_t*)p_csInfo;
    if (csInfo == NULL)
    {
        rcvdQty = -1;
        errno = EINVAL;
    }

    else if ((rcvdQty =
        recvfrom(csInfo->socketFd, inbuf, requestedQty, 0,//MSG_WAITALL,
                 (struct sockaddr *)&tmpAddr, &tmpAddrLength)) == -1)
    {
        if (errno != EAGAIN)
        {
		printf("unexpected error from recvfrom() - errno=%d '%s'\n", errno, strerror(errno));
        }
    }
    else if (rcvdQty != requestedQty)
    {
        printf("rcvd truncated datagram - ignoring - received %d bytes, "
            "expected %d\n", rcvdQty, requestedQty);
    }
    else
    {
        if (csInfo->inServerMode)
        {
            // Setup host addr for the next write
            csInfo->hostAddr = tmpAddr;
            csInfo->hostAddrLength = tmpAddrLength;
        }

        csInfo->msgsRcvdQty++;

        if (trace)
        {
            char hostStr[INET_ADDRSTRLEN];

#ifdef LINUX
	    inet_ntop(AF_INET, &tmpAddr.sin_addr, hostStr, sizeof(hostStr));
#else
            sprintf(hostStr, "%s", inet_ntoa(*(struct in_addr *)&tmpAddr.sin_addr));
#endif
	    printf("msg #%d: rcvd %d bytes from %s from port %d\n",
                csInfo->msgsRcvdQty, rcvdQty, hostStr, ntohs(tmpAddr.sin_port));
        }
    }

    return (rcvdQty);
}


int csWrite(csInfo_t* p_csInfo, void* outbuf, ssize_t requestedQty)
{
    ssize_t            sentQty;


    csRealInfo_t* csInfo = (csRealInfo_t*)p_csInfo;
    if (csInfo == NULL)
    {
        sentQty = -1;
        errno = EINVAL;
    }

    else if ((sentQty = sendto(
        csInfo->socketFd, outbuf, requestedQty, 0,
        (const struct sockaddr*)&csInfo->hostAddr,
        csInfo->hostAddrLength)) == -1)
    {
        static int skipCount = 0;

        if (--skipCount <= 0)
        {
            char hostStr[INET_ADDRSTRLEN];

#ifdef LINUX
	    inet_ntop(
                AF_INET, &csInfo->hostAddr.sin_addr, hostStr, sizeof(hostStr));
#else
	    sprintf(hostStr, "%s", inet_ntoa(*(struct in_addr *)&csInfo->hostAddr.sin_addr));
#endif
	    printf("#%d err: sendto(%d, ptr, %d, 0, (%d, %d, %s), %d): %s\n",
                csInfo->msgsSentQty, (int)csInfo->socketFd, requestedQty,
                csInfo->hostAddr.sin_family, ntohs(csInfo->hostAddr.sin_port),
				hostStr, csInfo->hostAddrLength, strerror(errno));
            skipCount = 100;
        }
    }

    else if (sentQty != requestedQty)
    {
        printf("sent truncated datagram - sent %d, expected %d\n",
            sentQty, requestedQty);
        sentQty = -1;
    }

    else
    {
        csInfo->msgsSentQty++;

        if (trace)
        {
            char hostStr[INET_ADDRSTRLEN];

#ifdef LINUX
	    inet_ntop(
                AF_INET, &csInfo->hostAddr.sin_addr, hostStr, sizeof(hostStr));
#else
	    sprintf(hostStr, "%s", inet_ntoa(*(struct in_addr *)&csInfo->hostAddr.sin_addr));
#endif
	    printf("msg #%d: sent %d bytes to %s on port %d\n",
                csInfo->msgsSentQty, sentQty, hostStr,
                ntohs(csInfo->hostAddr.sin_port));
        }
    }

    return (sentQty);
}


unsigned csGetSentQty(csInfo_t* p_csInfo)
{
    return ((p_csInfo == NULL) ? 0 : ((csRealInfo_t*)p_csInfo)->msgsSentQty);
}


unsigned csGetRcvdQty(csInfo_t* p_csInfo)
{
    return ((p_csInfo == NULL) ? 0 : ((csRealInfo_t*)p_csInfo)->msgsRcvdQty);
}


void csClose(csInfo_t* p_csInfo)
{
    csRealInfo_t* csInfo = (csRealInfo_t*)p_csInfo;
    if (csInfo != NULL)
    {
        if (csInfo->socketFd != -1)
			closesocket(csInfo->socketFd);
        csInfo->socketFd = -1;
        free(csInfo);
		csInfo = NULL;
    }
}
