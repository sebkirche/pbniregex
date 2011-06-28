// Main module for PbniRegexp
//
// @author : Sebastien Kirche - 2008, 2009, 2011

#include "PbniRegex.h"
#include "Main.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_all, LPVOID lpReserved)
{	
	switch(ul_reason_for_all){	
		case DLL_PROCESS_ATTACH:
			OutputDBSIfDebug(STR("PbniRegex Dll :: DllMain(DLL_PROCESS_ATTACH)\n"));
			break;
		case DLL_THREAD_ATTACH:
			OutputDBSIfDebug(STR("PbniRegex Dll :: DllMain(DLL_THREAD_ATTACH)\n"));
			break;
		case DLL_THREAD_DETACH:
			OutputDBSIfDebug(STR("PbniRegex Dll :: DllMain(DLL_THREAD_DETACH)\n"));
			break;
		case DLL_PROCESS_DETACH:
			OutputDBSIfDebug(STR("PbniRegex Dll :: DllMain(DLL_PROCESS_DETACH)\n"));
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
		/* uo_regex object */
		STR("class uo_regex from nonvisualobject\n")
		STR("   function string of_hello()\n")
		STR("   function string pcreversion()\n") 
		STR("   function boolean initialize(string as_pattern, boolean ab_globalscope, boolean ab_casesensitive)\n")
		STR("   function boolean test(string teststring)\n")
		STR("   subroutine setutf8(boolean isutf)\n")
		STR("   function long search(string searchstring)\n")
		STR("   function long matchcount()\n")
		STR("   function long matchposition(long al_index)\n")
		STR("   function long matchlength(long al_index)\n")
		STR("   function long groupcount(long al_matchindex)\n")
		STR("   function string match(long al_index)\n")
		STR("   function string group(long al_matchindex, long al_groupindex)\n")
		STR("   function string replace(string as_searchstring, string as_replacestring)\n")
		STR("   function long groupposition(long al_matchindex, long al_groupindex)\n")
		STR("   function long grouplength(long al_matchindex, long al_groupindex)\n")
		STR("   subroutine setmultiline(boolean ismulti)\n")
		STR("   function boolean ismultiline()\n")
		STR("   function boolean isutf8()\n")
		STR("   function boolean study()\n")
		STR("   function boolean getdotmatchnewline()\n")
		STR("   subroutine setdotmatchnewline(boolean match)\n")
		STR("   function boolean getextendedsyntax()\n")
		STR("   subroutine setextendedsyntax(boolean extended)\n")
		STR("   function boolean getungreedy()\n")
		STR("   subroutine setungreedy(boolean greedy)\n")
		STR("   function string getpattern()\n")
		STR("   function string getlasterror()\n")
#ifdef _DEBUG
		STR("   function string stringtest(string str)\n") 
#endif
		STR("end class\n")
		/* functions */
		STR("globalfunctions \n")
		STR("   function string fastreplaceall(string as_source, string as_pattern, string as_replace)\n")
		STR("   function string fastreplaceall2(string as_source, string as_pattern, string as_replace, boolean ab_casesensitive)\n")
		STR("end globalfunctions \n")
	};
	return (LPCTSTR)classDesc;
}

// PBX_CreateNonVisualObject is called by PowerBuilder to create a C++ class
// that corresponds to the PBNI class created by PowerBuilder.
PBXEXPORT PBXRESULT PBXCALL PBX_CreateNonVisualObject(IPB_Session * session, pbobject obj, LPCTSTR className, IPBX_NonVisualObject ** nvobj)
{
	// The name must not contain upper case
	if (_tccmp(className, STR("uo_regex")) == 0)
		*nvobj = new PbniRegex(session);
	return PBX_OK;
}


PBXEXPORT PBXRESULT PBXCALL PBX_InvokeGlobalFunction(IPB_Session* pbsession, LPCTSTR functionName, PBCallInfo* ci)
{
	PBXRESULT pbxr = PBX_E_NO_SUCH_CLASS;

	if (_tccmp(functionName, STR("fastreplaceall")) == 0){
		PbniRegex *regex = new PbniRegex(pbsession) ;
		pbxr = regex->FastReplace(ci) ;
 		if (regex != NULL) delete regex ;
		return PBX_OK ;
	}
	else if(_tccmp(functionName, STR("fastreplaceall2")) == 0){
		PbniRegex  *regex = new PbniRegex(pbsession) ;
		pbxr = regex->FastReplaceChooseCase(ci) ;
 		if (regex != NULL) delete regex ;
		return PBX_OK ;
	}

	return pbxr ;
}

