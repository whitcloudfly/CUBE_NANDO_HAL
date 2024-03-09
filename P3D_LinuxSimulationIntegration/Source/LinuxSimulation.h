// Copyright (c) 2010-2022 Lockheed Martin Corporation. All rights reserved.
// Use of this file is bound by the PREPAR3D® SOFTWARE DEVELOPER KIT END USER LICENSE AGREEMENT
    
// LinuxSimulation.h
// Description: A sample demonstrating how to have external network packets drive an ISimObject position.

#pragma once

#include "Simulation.h"

#include "ISimObject.h"
#include "Helpers\IUnknownHelper.h"

/************************************************************************************************
ISimObject parent class
************************************************************************************************/
// Ensure this GUID matches the CategoryId in the sim.cfg
class __declspec(uuid("{20947ED6-F9C1-422B-A180-89C40E0BA098}")) LinuxSimulation : public P3D::ISimObjectV03, P3D::IAirplaneServiceV01, P3D::IAvatarAttachServiceV01
{
public:

    DECLARE_IUNKNOWN_WITH_INLINE_REFCOUNT_IMPL();
    
    STDMETHOD(QueryBaseObject)(__in REFIID riid, void** ppv) override;
    STDMETHOD(QueryService)(__in REFGUID guidService, __in REFIID riid, void **ppv) override;

    //ISimObject implementations
    STDMETHOD(LoadConstantData)(__out void** ppConstantData)        override { return S_OK; }
    STDMETHOD(UnloadConstantData)(__inout void** ppConstantData)    override { return S_OK; }
    
    STDMETHOD(LoadDynamicData)() override { return S_OK; }

    STDMETHOD(Init)()   override { return m_Simulation.Init(); }  //e.g. init relationships between subsystems
    STDMETHOD(DeInit)() override { return S_OK; }

    STDMETHOD_(BOOL, SupportsLabels)()              const override { return m_bSupportsLabel; }                         //Does this simobject support label display?
    STDMETHOD (SetSupportsLabels)(BOOL bOn)               override { m_bSupportsLabel = (bOn != FALSE); return S_OK; }  //Should this simobject support label display?
    STDMETHOD_(void, OnModeChange)(int bfMode)            override { }                                                  //Called when a mode has changed, with the new mode bitfield
    STDMETHOD_(void, OnPositionInit)()                    override { }                                                  //Called upon change in non-real time object positions   (e.g. UI, slew)
    STDMETHOD_(void, OnSpeedInit)(float fSpeed)           override { }                                                  //(Feet per Second) Called upon change in speed   (e.g. UI)
    STDMETHOD_(float, GetIndicatedAirspeed)()       const override { return 0.0f; };              //Feet per second

    STDMETHOD_(BOOL, CanAvatarAttach()) const override { return m_Simulation.IsOnGround(); }
    STDMETHOD_(BOOL, CanAvatarDetach()) const override { return true; }

    //Factory function
    static HRESULT New(__in __notnull P3D::IBaseObjectV400* pBaseObject, __out __notnull P3D::ISimObject** ppThisSim);

    const P3D::IBaseObjectV400& GetBaseObject() const {return m_BaseObject;}
    
private:

    LinuxSimulation(P3D::IBaseObjectV400& BaseObject);
    ~LinuxSimulation();

    P3D::IBaseObjectV400& m_BaseObject;

    Simulation m_Simulation;
    BOOL m_bSupportsLabel;
};

//Unique class Guid
#define CLSID_LinuxSimulation __uuidof(LinuxSimulation)
REFGUID GetClassId() { return CLSID_LinuxSimulation; }

//Factory function wrapper
HRESULT New(__in __notnull P3D::IBaseObjectV400* pBaseObject, __out __notnull P3D::ISimObject** ppThisSim);
