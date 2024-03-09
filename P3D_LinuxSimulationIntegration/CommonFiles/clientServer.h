// Copyright (c) 2010-2022 Lockheed Martin Corporation. All rights reserved.
// Use of this file is bound by the PREPAR3D® SOFTWARE DEVELOPER KIT END USER LICENSE AGREEMENT
    
// ClientServer.h
// Description: Opens either a host or client connection.

#ifndef clientServer_H
#define clientServer_H

#define TRUE 1
#define FALSE 0

#include <stdio.h>

#ifdef __cplusplus
/* C bindings */
extern "C"
{
#endif /* __cplusplus */

typedef void* csInfo_t;
#ifndef LINUX
typedef int ssize_t;
#endif
/* 
 * Open a network connection to communicate between a client or server. The
 * nature of client or server is based on how the open is called - the server
 * passes NULL for host name and the port number that
 * it will be listening on. The client passes the name
 * of the server machine and the same port number.
 * Returns NULL if an error occurred or a pointer to a csInfo_t which must
 * be given to any further calls (csRead(), csWrite(), etc.).
 */
csInfo_t* csOpen(char* hostname, int portnum);

/*
 * Sets the socket to blocking (default) or non-blocking.  For non-blocking,
 * csRead() returns -1 if there is no UDP packet available.
 */
int csSetNonBlockingState(csInfo_t* p_csInfo, int isNonBlocking);

/*
 * Returns the number of bytes currently queued for reading.
 */
int csGetQueuedQty(csInfo_t* p_csInfo);

/*
 * Reads a packet if available.  If set to non-blocking, will return -1 if
 * no packet is available, otherwise will block until a packet is received.
 * Returns number of bytes read or -1 if an error occurred (errno will be
 * set to EAGAIN if non-blocking is set and no packet is available to be
 * read).
 */
int csRead(csInfo_t* p_csInfo, void* inbuf, ssize_t requestedQty);

/*
 * Writes a packet.  If in client mode, the packet will be sent to host given
 * in csOpen().  If in server mode, the packet will be sent to the client who
 * we mostly recently successfully read a packet from.
 * Returns number of bytes written or -1 if an error occurred.
 */
int csWrite(csInfo_t* p_csInfo, void* outbuf, ssize_t requestedQty);

/*
 * Returns the number of bytes sent since this port was opened.
 */
unsigned csGetSentQty(csInfo_t* p_csInfo);

/*
 * Returns the number of bytes received since this port was opened.
 */
unsigned csGetRcvdQty(csInfo_t* p_csInfo);

/*
 * Closes the network connection and frees resources. 
 */
void csClose(csInfo_t* p_csInfo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* clientServer_H */
