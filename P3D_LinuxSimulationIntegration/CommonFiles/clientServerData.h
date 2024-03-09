// Copyright (c) 2010-2022 Lockheed Martin Corporation. All rights reserved.
// Use of this file is bound by the PREPAR3D® SOFTWARE DEVELOPER KIT END USER LICENSE AGREEMENT
    
// ClientServerData.h
// Description: Sends/receives packet data to/from the host/client.

#include "clientServer.h"

#define INTERFACE_PORT 23332
#define HEARTBEAT_MAX 250

class HOST_PACKET_HEADER
{
public:
	enum PACKETS
	{
        //Host to P3D packets
        HOST2P3D_OWNSHIP_POSITION,
        HOST2P3D_ARTICULATIONS,

        //P3D to Host packets
        P3D2HOST_RADARALT,

        //Always leave this last
        PACKET_TYPES
	};
	unsigned long long int packet_magic;
	PACKETS packet_type;
	unsigned int packet_size;

	HOST_PACKET_HEADER();
	bool TestMagic();
};

class HOST_PACKET_OWNSHIP_POSITION : public HOST_PACKET_HEADER
{
public:
	HOST_PACKET_OWNSHIP_POSITION();
	double Latitude;   //Radians
	double Longitude;  //Radians
	double Altitude;   //Feet
	double Pitch;      //Radians
	double Bank;       //Radians
	double Heading;    //Radians
};

class HOST_PACKET_ARTICULATIONS : public HOST_PACKET_HEADER
{
public:
	HOST_PACKET_ARTICULATIONS();
	double WingPosition;
	double RudderPosition;
};

class HOST_PACKET_RADARALT : public HOST_PACKET_HEADER
{
public:
	HOST_PACKET_RADARALT();
	float RadarAlt;
};

class PACKET_SYSTEM
{
public:
	PACKET_SYSTEM();
	~PACKET_SYSTEM();
	HOST_PACKET_OWNSHIP_POSITION ownshipPosition;
	HOST_PACKET_ARTICULATIONS articulations;
	HOST_PACKET_RADARALT radarAlt;
	bool OpenPort(char* hostname, bool isP3D);
	bool ClosePort();
	bool WritePort();
	bool ReadPort();
    bool GetActive();
    bool GetConnected();
	bool OwnshipPositionUpdate();
	bool ArticulationsUpdate();
    bool RadarAltUpdate();
private:
	csInfo_t *csInfo;
	bool connected;
    bool active;
	bool sendall;
	int heartbeat_counter;
	bool IsP3D;
	HOST_PACKET_OWNSHIP_POSITION ownshipPositionHist;
	HOST_PACKET_ARTICULATIONS articulationsHist;
	HOST_PACKET_RADARALT radarAltHist;
    bool SetUpdate(HOST_PACKET_HEADER::PACKETS packetType);
	bool SendP3DToHost(char* data, int* size, int maxsize);
	bool ReceiveP3DToHost(char* data, int size);
	bool SendHostToP3D(char* data, int* size, int maxsize);
	bool ReceiveHostToP3D(char* data, int size);

	//Update Flags
	bool ownshipPositionUpdate;
	bool articulationsUpdate;
    bool radarAltUpdate;
};
