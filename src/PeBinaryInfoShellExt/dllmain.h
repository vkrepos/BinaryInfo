// dllmain.h : Declaration of module class.

class CPeBinaryInfoShellExtModule : public ATL::CAtlDllModuleT< CPeBinaryInfoShellExtModule >
{
public :
	DECLARE_LIBID(LIBID_PeBinaryInfoShellExtLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_PEBinaryInfoSHELLEXT, "{D9FCC852-BFB1-4946-8254-CF537EEDE761}")
};

extern class CPeBinaryInfoShellExtModule _AtlModule;
