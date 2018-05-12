// PeBinaryInfoShellExt.h : Declaration of the CPeBinaryInfoShellExt

#pragma once
#include "resource.h"       // main symbols



#include "PeBinaryInfoShellExt_i.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;


// CPeBinaryInfoShellExt

class ATL_NO_VTABLE CPeBinaryInfoShellExt :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPeBinaryInfoShellExt, &CLSID_PeBinaryInfoShellExt>,
	public IShellExtInit,
	public IShellPropSheetExt
{
public:
	CPeBinaryInfoShellExt()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_PEBinaryInfoSHELLEXT)

	DECLARE_NOT_AGGREGATABLE(CPeBinaryInfoShellExt)

	BEGIN_COM_MAP(CPeBinaryInfoShellExt)
		COM_INTERFACE_ENTRY(IShellExtInit)
		COM_INTERFACE_ENTRY(IShellPropSheetExt)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

	STDMETHODIMP Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

	STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE, LPARAM);

	STDMETHODIMP ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM)
	{
		return E_NOTIMPL;
	}

private:
	std::wstring m_filePath;
};

OBJECT_ENTRY_AUTO(__uuidof(PeBinaryInfoShellExt), CPeBinaryInfoShellExt)
