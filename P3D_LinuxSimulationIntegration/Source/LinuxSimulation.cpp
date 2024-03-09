// Copyright (c) 2010-2022 Lockheed Martin Corporation. All rights reserved.
// Use of this file is bound by the PREPAR3D® SOFTWARE DEVELOPER KIT END USER LICENSE AGREEMENT
    
// LinuxSimulation.cpp
// Description: A sample demonstrating how to have external network packets drive an ISimObject position.

#include "LinuxSimulation.h"

#include <atlcomcli.h>
#include <InitGuid.h>

using namespace P3D;

/**********************************************************************
**********************************************************************/
LinuxSimulation::LinuxSimulation(IBaseObjectV400& BaseObject) :
    m_RefCount(1),
    m_BaseObject(BaseObject), 
    m_Simulation(BaseObject)
{
}
    
/**********************************************************************
**********************************************************************/
LinuxSimulation::~LinuxSimulation()
{
}

/**********************************************************************
**********************************************************************/
STDMETHODIMP LinuxSimulation::QueryInterface(__in REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;

    if (!ppv)
    {
        goto Error;
    }
    
    *ppv = NULL;

    if (IsEqualIID(riid, IID_ISimObject))
    {
        *ppv = static_cast<ISimObject*>(this);
    }
    else if (IsEqualIID(riid, IID_ISimObjectV01))
    {
        *ppv = static_cast<ISimObjectV01*>(this);
    }
    else if (IsEqualIID(riid, IID_ISimObjectV02))
    {
        *ppv = static_cast<ISimObjectV02*>(this);
    }
    else if (IsEqualIID(riid, IID_ISimObjectV03))
    {
        *ppv = static_cast<ISimObjectV03*>(this);
    }
    else if (IsEqualIID(riid , IID_IAircraftService))
    {
        *ppv = static_cast<IAircraftService*>(this);
    }
    else if (IsEqualIID(riid , IID_IAircraftServiceV01))
    {
        *ppv = static_cast<IAircraftServiceV01*>(this);
    }
    else if (IsEqualIID(riid , IID_IAirplaneServiceV01))
    {
        *ppv = static_cast<IAirplaneServiceV01*>(this);
    }
    else if (IsEqualIID(riid , IID_IAvatarAttachServiceV01))
    {
        *ppv = static_cast<IAvatarAttachServiceV01*>(this);
    }        
    else if (IsEqualIID(riid , IID_IServiceProvider))
    {
        *ppv = static_cast<IServiceProvider*>(this);
    }
    else if (IsEqualIID(riid , IID_IUnknown))
    {
        *ppv = static_cast<LinuxSimulation*>(this);
    }        

    if (*ppv)
    {
        hr = S_OK;
        AddRef();
    }

Error:

    return hr;
}

/**********************************************************************
**********************************************************************/
STDMETHODIMP LinuxSimulation::QueryBaseObject( __in REFIID riid, void** ppv)
{
    HRESULT hr = E_NOINTERFACE;

    if (!ppv)
    {
        goto Error;
    }

    *ppv = NULL;
    
    hr = m_BaseObject.QueryInterface(riid, (void**)ppv);

Error:

    return hr;
}

/**********************************************************************
**********************************************************************/
STDMETHODIMP LinuxSimulation::QueryService(__in REFGUID guidService, __in REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;

    if (!ppv)
    {
        goto Error;
    }

    *ppv = NULL;

    if (IsEqualIID(guidService, SID_SimObject))
    {
        hr = QueryInterface(riid, ppv);
    }
    else if (IsEqualIID(guidService, SID_RotorcraftService))
    {
        hr = QueryInterface(riid, ppv);
    }
    else if (IsEqualIID(guidService, SID_AircraftService))
    {
        hr = QueryInterface(riid, ppv);
    }
    else if (IsEqualIID(guidService, SID_AvatarAttachService))
    {
        hr = QueryInterface(riid, ppv);
    }
    else if (IsEqualIID(guidService, SID_Simulation))
    {
        hr = m_Simulation.QueryInterface(riid, ppv);
    }

Error:

    return hr;
}

/**********************************************************************
Factory function
**********************************************************************/
/*static*/
HRESULT LinuxSimulation::New(__notnull IBaseObjectV400* pBaseObject, __notnull ISimObject** ppThisSim)
{
    HRESULT hr = E_FAIL;

    if (!ppThisSim)
    {
        goto Error;
    }

    *ppThisSim = new LinuxSimulation(*pBaseObject);

    if (!*ppThisSim)
    {
        goto Error;
    }

    hr = pBaseObject->RegisterSimulation(&static_cast<LinuxSimulation*>(*ppThisSim)->m_Simulation, 60.0f /*Hz*/);

Error:

    return hr;
}

/**********************************************************************
Factory function wrapper
**********************************************************************/
/*static*/
HRESULT New(__notnull IBaseObjectV400* pBaseObject, __notnull ISimObject** ppThisSim)
{
    CComPtr<IBaseObjectV400> spBaseObject;
    if (SUCCEEDED(pBaseObject->QueryInterface(IID_IBaseObjectV400, (void**)&spBaseObject)))
    {
        return LinuxSimulation::New(spBaseObject, ppThisSim);
    }
    else
    {
        return E_FAIL;
    }
}

/**********************************************************************
Property Registration
**********************************************************************/
HRESULT RegisterProperties(__notnull ISimObjectManagerV400* pSimObjectMgr)
{
    HRESULT hr = E_FAIL;

    hr = Simulation::RegisterProperties(CLSID_LinuxSimulation, pSimObjectMgr);

    return S_OK;
}
