#ifndef OMH_SHELL_EXT_H
#define OMH_SHELL_EXT_H

#include <windows.h>
#include <shlobj.h>     // For IShellExtInit and IContextMenu
#include <stdint.h>

class OMHShell : public IShellExtInit, public IContextMenu
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
	
    // IShellExtInit
    IFACEMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID);

    // IContextMenu
    IFACEMETHODIMP QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
    IFACEMETHODIMP GetCommandString(UINT_PTR idCommand, UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax);

	explicit OMHShell();
	~OMHShell();

private:
    // Reference count of component.
    long m_cRef;

	HBITMAP bmpMenu;
	//HICON hIcon;

	LPWSTR szMenuText_Main_First;
	LPWSTR szMenuText_Main_Second;
	LPWSTR szMenuText_Share;
	LPWSTR szMenuText_ShareLink;
	LPWSTR szMenuText_Notify;

	bool isSubMenu = false;

    // The name of the selected file.
    wchar_t m_szSelectedFile[MAX_PATH];

	enum : UINT{
		FIRST = 0,

		SHARE = FIRST,
		SENDLINK,
		NOTIFYME,
		MOVETO,

		LAST
	};

	// Commands for items
	void Command(HWND handle, wchar_t* message);

	// For verifying our folder
	bool CompareDirectory(LPCWSTR source, LPCWSTR destination);

	// Convert icon to bmp
	HBITMAP icon_to_bitmap(HICON Icon_Handle);
};


#endif // OMH_SHELL_EXT_H
