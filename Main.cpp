// Main module for PbniRegexp
//
// @author : Sebastien Kirche - 2008

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
#ifdef _DEBUG
		OutputDebugString(_T("PbniRegex Dll :: DllMain(DLL_PROCESS_ATTACH)"));
#endif
		break;
	case DLL_THREAD_ATTACH:
#ifdef _DEBUG
		OutputDebugString(_T("PbniRegex Dll :: DllMain(DLL_THREAD_ATTACH)"));
#endif
		break;
	case DLL_THREAD_DETACH:		
#ifdef _DEBUG
		OutputDebugString(_T("PbniRegex Dll :: DllMain(DLL_THREAD_DETACH)"));
#endif
		break;
	case DLL_PROCESS_DETACH:
#ifdef _DEBUG
		OutputDebugString(_T("PbniRegex Dll :: DllMain(DLL_PROCESS_DETACH)"));
#endif
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
      _T("   function string pcreversion()\n") \
      _T("   function boolean initialize(string as_pattern, boolean ab_globalscope, boolean ab_casesensitive)\n") \
      _T("   function boolean test(string teststring)\n") \
      _T("   subroutine setutf8(boolean isutf)\n") \
      _T("   function long search(string searchstring)\n") \
      _T("   function long matchcount()\n") \
      _T("   function long matchposition(long al_index)\n") \
      _T("   function long matchlength(long al_index)\n") \
      _T("   function long groupcount(long al_matchindex)\n") \
      _T("   function string match(long al_index)\n") \
      _T("   function string group(long al_matchindex, long al_groupindex)\n") \
      _T("   function string replace(string as_searchstring, string as_replacestring)\n") \
      _T("   function long groupposition(long al_matchindex, long al_groupindex)\n") \
      _T("   function long grouplength(long al_matchindex, long al_groupindex)\n") \
      _T("   subroutine setmultiline(boolean ismulti)\n") \
      _T("   function boolean ismultiline()\n") \
      _T("   function boolean isutf8()\n") \
	  _T("   function boolean study()\n") \
      _T("end class\n") \
      _T("globalfunctions \n") \
      _T("   function string fastreplaceall(string as_source, string as_pattern, string as_replace)\n") \
      _T("   function string fastreplaceall2(string as_source, string as_pattern, string as_replace, boolean ab_casesensitive)\n") \
      _T("end globalfunctions \n")
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


PBXEXPORT PBXRESULT PBXCALL PBX_InvokeGlobalFunction
(
	IPB_Session* pbsession,
	LPCTSTR      functionName,
	PBCallInfo* ci
)
{
	PBXRESULT pbxr = PBX_E_NO_SUCH_CLASS;

	if ( wcscmp( functionName, _T("fastreplaceall") ) == 0 ){

		PbniRegex  *regex = new PbniRegex( pbsession ) ;
		pbxr = regex->FastReplace( ci ) ;
 		if ( regex != NULL ) delete regex ;
		return PBX_OK ;
	}
	else if( wcscmp( functionName, _T("fastreplaceall2") ) == 0 ){

		PbniRegex  *regex = new PbniRegex( pbsession ) ;
		pbxr = regex->FastReplaceChooseCase( ci ) ;
 		if ( regex != NULL ) delete regex ;
		return PBX_OK ;
	}

	return pbxr ;
}
