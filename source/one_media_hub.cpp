#include "one_media_hub.h"
#include "resource.h"
#include <strsafe.h>
#include <Shlwapi.h>
#include <cstdio>
#include <Windows.h>
#include <string>

extern HINSTANCE g_hInst;
extern long g_cDllRef;

OMHShell::OMHShell(void) : m_cRef(1), szMenuText_Main_First(L"OBH Tools"),
                          szMenuText_Main_Second(L"Move to OBH drive"), szMenuText_Share(L"Share..."),
						  szMenuText_ShareLink(L"&Send link..."), szMenuText_Notify(L"Notify me about updates...")
{
    g_cDllRef++;
	//hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCEW(IDI_ICON), IMAGE_ICON, 16, 16, 0);
	bmpMenu = (HBITMAP)LoadImage(g_hInst, MAKEINTRESOURCE(IDB_OK), IMAGE_BITMAP, 16, 16, 0);
}

OMHShell::~OMHShell(void)
{
    g_cDllRef--;
}

// Query to the interface the component supported.
IFACEMETHODIMP OMHShell::QueryInterface(REFIID riid, void **ppv)
{
	if (ppv == 0)
		return E_POINTER;

	*ppv = NULL;

	if (IsEqualIID(riid, IID_IShellExtInit) || IsEqualIID(riid, IID_IUnknown)) {
		*ppv = static_cast<LPSHELLEXTINIT>(this);
	}
	else if (IsEqualIID(riid, IID_IContextMenu)) {
		*ppv = static_cast<LPCONTEXTMENU>(this);
	}
	else
	{
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

IFACEMETHODIMP_(ULONG) OMHShell::AddRef()
{
    return ++m_cRef;
}

IFACEMETHODIMP_(ULONG) OMHShell::Release()
{
    ULONG cRef = --m_cRef;
    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}


bool OMHShell::CompareDirectory(LPCWSTR source, LPCWSTR destination )
{
	std::wstring lineS(source);
	std::wstring lineD(destination);
	bool result = (lineS.size() > lineD.size()) && (lineS.compare(0, lineD.size(), lineD) == 0);
	return result;
}

STDMETHODIMP OMHShell::Initialize(LPCITEMIDLIST pidl, LPDATAOBJECT pDataObj, HKEY hk)
{

	if (pDataObj == NULL)
		return E_INVALIDARG;

	pDataObj->AddRef();

	STGMEDIUM   medium;
	FORMATETC   fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	HRESULT hr = pDataObj->GetData(&fe, &medium);
	if (FAILED(hr))
		return E_INVALIDARG;

	// save the file name
	if (DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF, NULL, 0) == 1)
	{
		DragQueryFile((HDROP)medium.hGlobal, 0, m_szSelectedFile,
			sizeof(m_szSelectedFile));
		
		// this is indicator our folder "C:\Users\m.tomashivskyi\Documents\OneMediaHub"
		isSubMenu = CompareDirectory(m_szSelectedFile, L"C:\\Users\\m.tomashivskyi\\Documents\\OneMediaHub");
		hr = NOERROR;

	}
	else
		hr = E_INVALIDARG;

	ReleaseStgMedium(&medium);

	return hr;

}

IFACEMETHODIMP OMHShell::QueryContextMenu(
	HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	UINT idCmd = idCmdFirst;
	MENUITEMINFO mii = { sizeof(mii) };

	InsertMenu(hMenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

	if (isSubMenu)
	{
		HMENU hSubMenu = CreateMenu();

		mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_ID;
		mii.wID = idCmd + SHARE;
		mii.fType = MFT_STRING;
		mii.dwTypeData = szMenuText_Share;
		mii.fState = MFS_ENABLED;
		mii.hbmpItem = bmpMenu;
		if (hSubMenu)
		{
			InsertMenuItem(hSubMenu, 0, TRUE, &mii);

			mii.wID = idCmd + SENDLINK;
			mii.dwTypeData = szMenuText_ShareLink;

			InsertMenuItem(hSubMenu, 1, TRUE, &mii);
			InsertMenu(hSubMenu, 2, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

			mii.wID = idCmd + NOTIFYME;
			mii.dwTypeData = szMenuText_Notify;

			InsertMenuItem(hSubMenu, 3, TRUE, &mii);
		}

		mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_SUBMENU;
		mii.dwTypeData = szMenuText_Main_First;

		//bmpMenu = icon_to_bitmap(hIcon);

		mii.hSubMenu = hSubMenu;

		InsertMenu(hMenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

		if (!InsertMenuItem(hMenu, indexMenu++, TRUE, &mii))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}
	}


	mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
	mii.wID = idCmdFirst + MOVETO;
	mii.fType = MFT_STRING;
	mii.dwTypeData = szMenuText_Main_Second;
	mii.fState = MFS_ENABLED;
	mii.hbmpItem = bmpMenu;

	if (!InsertMenuItem(hMenu, indexMenu++, TRUE, &mii))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	InsertMenu(hMenu, indexMenu++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

	return MAKE_HRESULT(SEVERITY_SUCCESS, 0, LAST);
}

void OMHShell::Command(HWND handle, wchar_t* message)
{
	if (message == nullptr)
		message = L"";
	TCHAR szMsg[MAX_PATH];
	wsprintf(szMsg, L"The selected item is  %s:\n%s", message, m_szSelectedFile);
	MessageBox(handle,
		szMsg,
		L"Message Box!",
		MB_ICONINFORMATION);
}

IFACEMETHODIMP OMHShell::InvokeCommand(LPCMINVOKECOMMANDINFO pCmdInfo)
{
	if (!HIWORD(pCmdInfo->lpVerb))
	{
		UINT idCmd = LOWORD(pCmdInfo->lpVerb);

		if (idCmd == SHARE)
		{
			Command(pCmdInfo->hwnd, L"Share");
		}
		else if (idCmd == SENDLINK)
		{
			Command(pCmdInfo->hwnd, L"Send link");
		}
		else if (idCmd == NOTIFYME)
		{
			Command(pCmdInfo->hwnd, L"Notify me about updates...");
		}
		else if (idCmd == MOVETO)
		{
			Command(pCmdInfo->hwnd, L"Move to OBH Drive");
		}

		return S_OK;
	}
	return E_INVALIDARG;
}

// 
IFACEMETHODIMP OMHShell::GetCommandString(UINT_PTR idCommand,
    UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
    return S_OK;
}

HBITMAP OMHShell::icon_to_bitmap(HICON Icon_Handle)
{
	HDC Screen_Handle = GetDC(NULL);
	HDC Device_Handle = CreateCompatibleDC(Screen_Handle);

	HBITMAP Bitmap_Handle =
		CreateCompatibleBitmap(Device_Handle, 16,
		16);

	HBITMAP Old_Bitmap = (HBITMAP)SelectObject(Device_Handle, Bitmap_Handle);
	DrawIcon(Device_Handle, 0, 0, Icon_Handle);
	SelectObject(Device_Handle, Old_Bitmap);

	DeleteDC(Device_Handle);
	ReleaseDC(NULL, Screen_Handle);
	return Bitmap_Handle;
}


