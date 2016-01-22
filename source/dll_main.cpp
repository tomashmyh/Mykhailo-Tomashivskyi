#include <windows.h>
#include <Guiddef.h>
#include "class_factory.h"           // For the class factory
#include "Reg.h"
#include <Winuser.h>
#include "resource.h"

const GUID CLSID_OBHShell =
{ 0x34854bdb, 0x2fc0, 0x4461, { 0x9c, 0x73, 0xad, 0x73, 0xab, 0x49, 0x83, 0xc8 } };

HINSTANCE   g_hInst     = NULL;
long        g_cDllRef   = 0;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
        g_hInst = hModule;
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	if (IsEqualCLSID(CLSID_OBHShell, rclsid))
    {
        hr = E_OUTOFMEMORY;

        ClassFactory *pClassFactory = new ClassFactory();
        if (pClassFactory)
        {
            hr = pClassFactory->QueryInterface(riid, ppv);
            pClassFactory->Release();
        }
    }

    return hr;
}

STDAPI DllCanUnloadNow(void)
{
    return g_cDllRef > 0 ? S_FALSE : S_OK;
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr;

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    // Register the component.
	hr = RegisterInprocServer(szModule, CLSID_OBHShell,
        L"Shell_Extensions.OMHShell Class", 
        L"Apartment");
    if (SUCCEEDED(hr))
    {
        // Register the context menu handler.
        RegisterShellExtContextMenuHandler(L"*", 
			CLSID_OBHShell,
            L"Shell_Extensions.OMHShell");
		hr = RegisterShellExtContextMenuHandler(L"Folder",
			CLSID_OBHShell,
			L"Shell_Extensions.OMHShell");
    }

	return S_OK;
}


STDAPI DllUnregisterServer(void)
{
    HRESULT hr = S_OK;

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

   // Unregister the component.
	hr = UnregisterInprocServer(CLSID_OBHShell);
    if (SUCCEEDED(hr))
    {
        // Unregister the context menu handler.
        UnregisterShellExtContextMenuHandler(L"*", 
			CLSID_OBHShell);
		hr = UnregisterShellExtContextMenuHandler(L"Folder",
			CLSID_OBHShell);
    }
	
    return hr;
}

