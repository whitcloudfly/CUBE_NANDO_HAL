// Copyright (c) 2010-2022 Lockheed Martin Corporation. All rights reserved.
// Use of this file is bound by the PREPAR3D® SOFTWARE DEVELOPER KIT END USER LICENSE AGREEMENT
    
// ClientServerData.cpp
// Description: Sends/receives packet data to/from the host/client.

#include "clientServerData.h"
#include <string.h>

HOST_PACKET_HEADER::HOST_PACKET_HEADER()
{
    packet_magic = 0xAC56E3A4903F74DD;
}

bool HOST_PACKET_HEADER::TestMagic()
{
    return (packet_magic == 0xAC56E3A4903F74DD);
}

HOST_PACKET_OWNSHIP_POSITION::HOST_PACKET_OWNSHIP_POSITION()
{
    packet_type = HOST_PACKET_HEADER::HOST2P3D_OWNSHIP_POSITION;
    packet_size = sizeof(*this);
}

HOST_PACKET_ARTICULATIONS::HOST_PACKET_ARTICULATIONS()
{
	packet_type = HOST_PACKET_HEADER::HOST2P3D_ARTICULATIONS;
	packet_size = sizeof(*this);
}

HOST_PACKET_RADARALT::HOST_PACKET_RADARALT()
{
	packet_type = HOST_PACKET_HEADER::P3D2HOST_RADARALT;
	packet_size = sizeof(*this);
}

PACKET_SYSTEM::PACKET_SYSTEM()
{
	heartbeat_counter = HEARTBEAT_MAX;
        connected = false;
	ownshipPositionUpdate = false;
	articulationsUpdate = false;
	radarAltUpdate = false;
}

PACKET_SYSTEM::~PACKET_SYSTEM()
{
	ClosePort();
}

bool PACKET_SYSTEM::OpenPort(char* hostname, bool isP3D)
{
    IsP3D = isP3D;
    connected = false;
    if ((csInfo = csOpen(hostname, INTERFACE_PORT)))
    {
        csSetNonBlockingState(csInfo, 1);
        connected = true;
    }
    return connected;
}

bool PACKET_SYSTEM::ClosePort()
{
    csClose(csInfo);
    return true;
}

bool PACKET_SYSTEM::WritePort()
{
    bool ret = false;
    char buffer[2000];
    int size = 0;
    if (connected)
    {
        if (IsP3D)
        {
            ret = SendP3DToHost(buffer, &size, sizeof(buffer));
        }
        else
        {
            ret = SendHostToP3D(buffer, &size, sizeof(buffer));
        }
        if (ret)
        {
            csWrite(csInfo, buffer, size);
        }
    }
    return ret;
}

bool PACKET_SYSTEM::ReadPort()
{
    bool ret = false;
    char buffer[2000];
    int size;
	heartbeat_counter++;
    while (connected && ((size = csGetQueuedQty(csInfo)) > 0))
    {
        if (size > (int)sizeof(buffer))
            size = sizeof(buffer);
        size = csRead(csInfo, buffer, size);
        if (IsP3D)
        {
            ret = ReceiveHostToP3D(buffer, size);
        }
        else
        {
            ret = ReceiveP3DToHost(buffer, size);
        }
        if(ret)
        {
			if (!GetActive())
				sendall = true;
			heartbeat_counter = 0;
        }
    }
    return ret;
}

bool PACKET_SYSTEM::GetConnected()
{
    return connected;
}

bool PACKET_SYSTEM::GetActive()
{
    return (heartbeat_counter < HEARTBEAT_MAX);
}

bool PACKET_SYSTEM::OwnshipPositionUpdate()
{
	bool ret = ownshipPositionUpdate;
	ownshipPositionUpdate = false;
	return ret;
}

bool PACKET_SYSTEM::ArticulationsUpdate()
{
	bool ret = articulationsUpdate;
	articulationsUpdate = false;
	return ret;
}

bool PACKET_SYSTEM::RadarAltUpdate()
{
	bool ret = radarAltUpdate;
	radarAltUpdate = false;
	return ret;
}

bool PACKET_SYSTEM::SetUpdate(HOST_PACKET_HEADER::PACKETS packetType)
{
    bool ret = true;
    switch(packetType)
    {
		case HOST_PACKET_HEADER::HOST2P3D_OWNSHIP_POSITION:
			ownshipPositionUpdate = true;
			ret = true;
			break;
		case HOST_PACKET_HEADER::HOST2P3D_ARTICULATIONS:
			articulationsUpdate = true;
			ret = true;
			break;
		case HOST_PACKET_HEADER::P3D2HOST_RADARALT:
			articulationsUpdate = true;
			ret = true;
			break;
        default:
            ret = false;
            break;
    }
    return ret;
}

bool PACKET_SYSTEM::SendP3DToHost(char* data, int* size, int maxsize)
{
    HOST_PACKET_HEADER *list[] = { &radarAlt, &radarAltHist };
    bool ret = false;
	if (sendall) //invalidate all histories;
	{
		for (unsigned int i = 0; i < sizeof(list) / sizeof(&list); i += 2)
		{
			memset(list[i + 1], 0, list[i]->packet_size);
		}
		sendall = false;
	}
	*size = 0;
    for (unsigned int i = 0; i < sizeof(list) / sizeof(&list); i += 2)
    {
		//Always send radarAlt packet as a keep alive
        if (list[i] == &radarAlt || memcmp(list[i], list[i + 1], list[i]->packet_size))
        {
            if (*size + (int)(list[i]->packet_size) > maxsize)
                break;
            ret = true;
            memcpy(list[i + 1], list[i], list[i]->packet_size);
            memcpy(data + *size, list[i], list[i]->packet_size);
            *size += list[i]->packet_size;
        }
    }
    return ret;
}

bool PACKET_SYSTEM::ReceiveP3DToHost(char* data, int size)
{
    HOST_PACKET_HEADER *list[] = { &radarAlt };
    int position = 0;
    bool ret = position < size;
    HOST_PACKET_HEADER hph;
    while (position < size && ret)
    {
        ret = false;
        memcpy(&hph, data + position, sizeof(hph));
        if (hph.TestMagic())
        {
            for (unsigned int i = 0; i < sizeof(list) / sizeof(&list); i++)
            {
                if (list[i]->packet_size == hph.packet_size && list[i]->packet_type == hph.packet_type)
                {
                    SetUpdate(hph.packet_type);
                    memcpy(list[i], data + position, list[i]->packet_size);
                    position += list[i]->packet_size;
                    ret = true;
                }
            }
        }
    }
    return ret;
}

bool PACKET_SYSTEM::SendHostToP3D(char* data, int* size, int maxsize)
{
    HOST_PACKET_HEADER *list[] = { &ownshipPosition,		&ownshipPositionHist,
				   &articulations,		&articulationsHist }; //Add additional packets here

    bool ret = false;
	if (sendall) //invalidate all histories;
	{
		for (unsigned int i = 0; i < sizeof(list) / sizeof(&list); i += 2)
		{
			memset(list[i + 1], 0, list[i]->packet_size);
		}
		sendall = false;
	}
	
	*size = 0;
    for (unsigned int i = 0; i < sizeof(list) / sizeof(&list); i += 2)
    {
		//Always send ownship position packet as a keep alive
		if (list[i] == &ownshipPosition || memcmp(list[i], (int*)list[i + 1], list[i]->packet_size))
        {
            if (*size + (int)(list[i]->packet_size) > maxsize)
                break;
            ret = true;
            memcpy(list[i + 1], list[i], list[i]->packet_size);
            memcpy(data + *size, list[i], list[i]->packet_size);
            *size += list[i]->packet_size;
        }
    }
    return ret;
}

bool PACKET_SYSTEM::ReceiveHostToP3D(char* data, int size)
{
    HOST_PACKET_HEADER *list[] = { &ownshipPosition,
				   &articulations }; //Add additional packets here
    int position = 0;
    bool ret = position < size;
    HOST_PACKET_HEADER hph;
    while (position < size && ret)
    {
        ret = false;
        memcpy(&hph, data + position, sizeof(hph));
        if (hph.TestMagic())
        {
            for ( unsigned int i = 0; i < sizeof(list) / sizeof(&list); i++)
            {
                if (list[i]->packet_size == hph.packet_size && list[i]->packet_type == hph.packet_type)
                {
                    SetUpdate(hph.packet_type);
                    memcpy(list[i], data + position, list[i]->packet_size);
                    position += list[i]->packet_size;
                    ret = true;
                }
            }
        }
    }
    return ret;
}
