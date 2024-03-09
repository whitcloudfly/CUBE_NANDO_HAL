// Copyright (c) 2010-2022 Lockheed Martin Corporation. All rights reserved.
// Use of this file is bound by the PREPAR3D® SOFTWARE DEVELOPER KIT END USER LICENSE AGREEMENT

// Simulation.cpp
// Description: A sample demonstrating how to have external network packets drive an ISimObject position.

#include <atlcomcli.h>
#include <initguid.h>

#include "Simulation.h"

#define MAKE_STATIC 1
#ifdef  MAKE_STATIC
#define STATIC_CONST static const
#else
#define STATIC_CONST 
#endif

using namespace P3D;

DEFINE_GUID(CLSID_Simulation, 0xdc89e031, 0x65a1, 0x4234, 0x9e, 0x62, 0x37, 0x62, 0x64, 0xaf, 0xe2, 0x18);

/****************************************************************
****************************************************************/
Simulation::Simulation(IBaseObjectV400& BaseObject) :
    m_RefCount(1),
    m_BaseObject(BaseObject),
    m_bOnGround(FALSE)
{
    memset(&m_SurfaceInfo, 0, sizeof(m_SurfaceInfo));
}

/****************************************************************
****************************************************************/
Simulation::~Simulation()
{
}

/****************************************************************
****************************************************************/
HRESULT Simulation::Init()
{
    p3dhost.OpenPort(NULL, TRUE);

    p3dhost.articulations.WingPosition = 0.0;
    p3dhost.articulations.RudderPosition = 0.0;

    return S_OK;
}

/****************************************************************
****************************************************************/
void Simulation::InitPositionFromBase()
{
    m_BaseObject.GetPosition(m_vdPos, m_vdOrient, m_vdVel, m_vdRotVel);
}

/****************************************************************
****************************************************************/
STDMETHODIMP Simulation::QueryInterface(__in REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;

    if (!ppv)
    {
        goto Error;
    }

    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (IsEqualIID(riid, IID_ISimulationV01))
    {
        *ppv = static_cast<ISimulationV01*>(this);
    }
    else if (IsEqualIID(riid, IID_ISimulation))
    {
        *ppv = static_cast<ISimulation*>(this);
    }
    else if (IsEqualIID(riid, CLSID_Simulation))
    {
        *ppv = static_cast<Simulation*>(this);
    }

    if (*ppv)
    {
        hr = S_OK;
        AddRef();
    }

Error:

    return hr;
}

/****************************************************************
****************************************************************/
STDMETHODIMP Simulation::Update(double dDeltaT)
{
    //Update surface info
    m_BaseObject.GetSurfaceInformation(m_SurfaceInfo, NULL);
    
    //Get current position
    m_BaseObject.GetPosition(m_vdPos, m_vdOrient, m_vdVel, m_vdRotVel);
    
    //Update Subsystems
    UpdateGroundReaction(dDeltaT);

    //Set data to write to Linux application
    p3dhost.radarAlt.RadarAlt = (float)m_vdPos.dY - m_SurfaceInfo.m_fElevation;

    //Write to port
    p3dhost.WritePort();

    //Read from port
    p3dhost.ReadPort();

    //Use data to modify ISimObject
    if (p3dhost.OwnshipPositionUpdate() && p3dhost.GetActive())
    {
        P3D::DXYZ vdPos = { p3dhost.ownshipPosition.Longitude, p3dhost.ownshipPosition.Altitude, p3dhost.ownshipPosition.Latitude };
        P3D::DXYZ vdOrient = { -p3dhost.ownshipPosition.Pitch, p3dhost.ownshipPosition.Heading, p3dhost.ownshipPosition.Bank };
        P3D::DXYZ vdVel = { 0.0, 0.0, 0.0 };
        P3D::DXYZ vdRotVel = { 0.0, 0.0, 0.0 };

        //Update Prepar3D object
        m_BaseObject.SetPosition(vdPos, vdOrient, vdVel, vdRotVel, m_bOnGround, dDeltaT);
    }

    return S_OK;
}

/****************************************************************
****************************************************************/
void Simulation::UpdateGroundReaction(double dDeltaT)
{
    STATIC_CONST float STATIC_HEIGHT_ABOVE_GROUND = 3.7f;

    double dHeightAboveGround = m_vdPos.dY - m_SurfaceInfo.m_fElevation - STATIC_HEIGHT_ABOVE_GROUND;

    if (dHeightAboveGround < 0)   // On ground
    {
        m_bOnGround = TRUE;
    }
    else //In air
    {
        m_bOnGround = FALSE;
    }
}

//Helper function to get from the ISimObject to the Simulation
static HRESULT SimulationGet(const __in ISimObject& Sim, __out Simulation** ppSimulation)
{
    if (!ppSimulation)
    {
        return E_POINTER;
    }
 
    return const_cast<ISimObject&>(Sim).QueryService(SID_Simulation, CLSID_Simulation, (void**)ppSimulation);
}

//Get property macro
#define SIMULATION_PROPERTY_GET(PROPNAME, GETFUNC)\
/*static*/ STDMETHODIMP Simulation::PROPNAME(__in const ISimObject& Sim, __out double& dProperty, __in int iIndex)\
{\
    Simulation* pSimulation;\
    if (SUCCEEDED(SimulationGet(Sim, &pSimulation)))\
    {\
        dProperty = pSimulation->GETFUNC();\
        pSimulation->Release();\
        return S_OK;\
    }\
    else\
    {\
        dProperty = 0;\
        return E_FAIL;\
    }\
}\

SIMULATION_PROPERTY_GET(GetWingPosition, GetWingPosition)
SIMULATION_PROPERTY_GET(GetRudderPosition, GetRudderPosition)

/****************************************************************
****************************************************************/
/*static*/
HRESULT Simulation::RegisterProperties(__in REFGUID guidCategory,
                                       __in __notnull P3D::ISimObjectManagerV400* pSimObjectMgr)
{
    pSimObjectMgr->RegisterProperty(guidCategory, TEXT("GetWingPosition"), TEXT("number"), Simulation::GetWingPosition);
    pSimObjectMgr->RegisterProperty(guidCategory, TEXT("GetRudderPosition"), TEXT("number"), Simulation::GetRudderPosition);

    return S_OK;
}
