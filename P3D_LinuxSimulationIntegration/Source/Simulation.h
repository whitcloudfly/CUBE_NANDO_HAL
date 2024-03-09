// Copyright (c) 2010-2022 Lockheed Martin Corporation. All rights reserved.
// Use of this file is bound by the PREPAR3D® SOFTWARE DEVELOPER KIT END USER LICENSE AGREEMENT
    
// Simulation.h
// Description: A sample demonstrating how to have external network packets drive an ISimObject position.

#pragma once

#include "ISimObject.h"
#include "Helpers\IUnknownHelper.h"

#include "..\CommonFiles\clientServerData.h"

#define DECLARE_PROPERTY_GET(PROPNAME) static STDMETHODIMP PROPNAME(__in const P3D::ISimObject& Sim, __out double& dProperty, __in int iIndex);

class __declspec(uuid("{EFB054B1-61FB-4F2A-818C-F78DF9676F43}")) Simulation : public P3D::ISimulationV01
{
public:

    Simulation(P3D::IBaseObjectV400& BaseObject);
    ~Simulation();

    HRESULT Init();

    void    InitPositionFromBase();

    double  GetWingPosition()        const { return p3dhost.articulations.WingPosition; }
    double  GetRudderPosition()      const { return p3dhost.articulations.RudderPosition; }
    BOOL    IsOnGround()             const { return m_bOnGround; }

    /*********** Static property handlers ***********/
    static HRESULT RegisterProperties(__in REFGUID guidCategory, __in __notnull P3D::ISimObjectManagerV400* pSimObjectMgr);

    DECLARE_PROPERTY_GET(GetWingPosition)
    DECLARE_PROPERTY_GET(GetRudderPosition)
    /*********** Static property handlers ***********/

private:

    DECLARE_IUNKNOWN_WITH_INLINE_REFCOUNT_IMPL();

    P3D::IBaseObjectV400& m_BaseObject;

    // ISimulationV01 - Begin
    STDMETHOD(SaveLoadState)(__in __notnull P3D::PSaveLoadCallback pfnCallback, __in const BOOL bSave) { return S_OK; }
    STDMETHOD(Update)(double dDeltaT);
    // ISimulationV01 - End

    void UpdateGroundReaction(double dDeltaT);

private:

    P3D::SurfaceInfoV400 m_SurfaceInfo;

    // World-relative state
    P3D::DXYZ m_vdPos;
    P3D::DXYZ m_vdOrient;
    P3D::DXYZ m_vdVel;
    P3D::DXYZ m_vdRotVel;

    BOOL m_bOnGround;

    PACKET_SYSTEM p3dhost;
};
