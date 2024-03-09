// Copyright (c) 2010-2022 Lockheed Martin Corporation. All rights reserved.
// Use of this file is bound by the PREPAR3D® SOFTWARE DEVELOPER KIT END USER LICENSE AGREEMENT
    
// DllMain.cpp
// Description: A sample demonstrating how to implement a simple airplane ISimObject.

#include <atlcomcli.h>

//From Prepar3D SDK
#include <Pdk.h>   
#include <InitGuid.h>
#include <ISimObject.h>

using namespace P3D;

extern REFGUID GetClassId();
extern HRESULT New(__in __notnull IBaseObjectV400* pBaseObject, __out __notnull ISimObject** ppThisSim);
extern HRESULT RegisterProperties(__in __notnull ISimObjectManagerV400* pSimObjectMgr);

static CComPtr<IPdk> s_pPdk;

/***********************************************************************************************
***********************************************************************************************/
HRESULT GetPdk(__in REFIID riid, __out __notnull void** ppPdk)
{
    if (!s_pPdk)
    {
        return E_POINTER;
    }

    return s_pPdk->QueryInterface(riid, (void**)ppPdk);
}

/***********************************************************************************************
***********************************************************************************************/
STDMETHODIMP_(void) DLLStart(__in __notnull IPdk* pPdk)
{      
    CComPtr<ISimObjectManagerV400> spSimObjectMgr;  //Smart pointer reference to SimObject Manager

    HRESULT hr = E_FAIL;
    
    if (!pPdk)
    {
        goto Error;
    }

    s_pPdk = pPdk;

    //Get the SimObject manager to register a simobject class and its create factory callback
    if (FAILED(pPdk->QueryService(SID_SimObjectManager, IID_ISimObjectManagerV400, (void**)&spSimObjectMgr)))
    {
        goto Error;
    }

    //Register class and factory callback function
    if (FAILED(spSimObjectMgr->RegisterSimulationCategory(GetClassId(),    //Unique class Guid
                                                          TEXT("Airplane"),    //Friendly classification
                                                          New)))           //Factory function
    {
        goto Error;
    }  

    if (FAILED(RegisterProperties(spSimObjectMgr)))
    {
        goto Error;
    }

Error:

    return;
}

/***********************************************************************************************
***********************************************************************************************/

STDMETHODIMP_(void) DLLStop()
{
    //Clean up statics on module unload as necessary
}

/***********************************************************************************************
***********************************************************************************************/
