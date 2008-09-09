// PbniHash.cpp : defines the entry point for the PBNI DLL.
//Global: 0
//Unicode: -1
#include "PbniRegex.h"
#include "Main.h"


BOOL APIENTRY DllMain
(
   HANDLE hModule,
   DWORD ul_reason_for_all,
   LPVOID lpReserved
)
{	
	switch(ul_reason_for_all)
	{	
	case DLL_PROCESS_ATTACH:
		OutputDebugString(_T("PbniRegex Dll :: DllMain(DLL_PROCESS_ATTACH)"));
		break;
	case DLL_THREAD_ATTACH:
		OutputDebugString(_T("PbniRegex Dll :: DllMain(DLL_THREAD_ATTACH)"));
		break;
	case DLL_THREAD_DETACH:		
		OutputDebugString(_T("PbniRegex Dll :: DllMain(DLL_THREAD_DETACH)"));
		break;
	case DLL_PROCESS_DETACH:
		OutputDebugString(_T("PbniRegex Dll :: DllMain(DLL_PROCESS_DETACH)"));
		break;
	}
	return TRUE;
}

// The description returned from PBX_GetDescription is used by
// the PBX2PBD tool to create pb groups for the PBNI class.
//
// + The description can contain more than one class definition.
// + A class definition can reference any class definition that
//   appears before it in the description.
// + The PBNI class must inherit from a class that inherits from
//   NonVisualObject, such as Transaction or Exception

PBXEXPORT LPCTSTR PBXCALL PBX_GetDescription()
{
   // combined class description
   static const TCHAR classDesc[] = {
      /* PbniHash */
      _T("class uo_regex from nonvisualobject\n") \
      _T("   function string of_hello()\n") \
      _T("   function string version()\n") \
      _T("   function boolean initialize(string as_pattern, boolean ab_globalscope, boolean ab_casesensitive)\n") \
      _T("   function boolean test(string teststring)\n") \
      _T("   subroutine setutf8(boolean isutf)\n") \
      _T("   function long search(string searchstring)\n") \
      _T("end class\n")
	};
   return (LPCTSTR)classDesc;
}

// PBX_CreateNonVisualObject is called by PowerBuilder to create a C++ class
// that corresponds to the PBNI class created by PowerBuilder.
PBXEXPORT PBXRESULT PBXCALL PBX_CreateNonVisualObject
(
   IPB_Session * session,
   pbobject obj,
   LPCTSTR className,
   IPBX_NonVisualObject ** nvobj
)
{
   // The name must not contain upper case
   if (_tcscmp(className, _T("uo_regex")) == 0)
      *nvobj = new PbniRegex(session);
   return PBX_OK;
}

