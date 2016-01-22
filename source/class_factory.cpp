#include "class_factory.h"
#include "one_media_hub.h"
#include <new>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")


extern long g_cDllRef;


ClassFactory::ClassFactory() : m_cRef(1)
{
    ++g_cDllRef;
}

ClassFactory::~ClassFactory()
{
    --g_cDllRef;
}


//
// IUnknown
//

IFACEMETHODIMP ClassFactory::QueryInterface(REFIID riid, void **ppv)
{
	if (ppv == 0)
		return E_POINTER;

	*ppv = NULL;

	if (IsEqualIID(riid, IID_IUnknown))
		*ppv = this;
	else
		if (IsEqualIID(riid, IID_IClassFactory))
			*ppv = (IClassFactory*)this;

	if (*ppv)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

IFACEMETHODIMP_(ULONG) ClassFactory::AddRef()
{
    return ++m_cRef;
}

IFACEMETHODIMP_(ULONG) ClassFactory::Release()
{
    ULONG cRef = --m_cRef;
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}


// 
// IClassFactory
//

IFACEMETHODIMP ClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = CLASS_E_NOAGGREGATION;
    if (pUnkOuter == NULL)
    {
        hr = E_OUTOFMEMORY;

        // Create the COM component.
		OMHShell *pExt = new (std::nothrow) OMHShell();
        if (pExt)
        {
            // Query the specified interface.
            hr = pExt->QueryInterface(riid, ppv);
            pExt->Release();
        }
    }

    return hr;
}

IFACEMETHODIMP ClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        g_cDllRef++;
    }
    else
    {
        g_cDllRef--;
    }
    return S_OK;
}