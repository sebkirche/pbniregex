// My.h : header file for PBNI class
#ifndef CPBNIREGEX_H
#define CPBNIREGEX_H

#include <pbext.h>
#include "pcre.h"

class PbniRegex : public IPBX_NonVisualObject
{
public:
	// construction/destruction
	PbniRegex();
	PbniRegex( IPB_Session * pSession );
	virtual ~PbniRegex();

	// IPBX_UserObject methods
	PBXRESULT Invoke
	(
		IPB_Session * session,
		pbobject obj,
		pbmethodID mid,
		PBCallInfo * ci
	);

   void Destroy();

	// PowerBuilder method wrappers
	enum Function_Entrys
	{
		mid_Hello = 0,
		mid_Init,
		mid_Test,
		mid_SetUtf,
		mid_Search,
		NO_MORE_METHODS
	};


protected:
 	// methods callable from PowerBuilder
	PBXRESULT Hello( PBCallInfo * ci );
	PBXRESULT Initialize(PBCallInfo * ci);
	PBXRESULT Test(PBCallInfo * ci);
	PBXRESULT SetUtf(PBCallInfo * ci);
	PBXRESULT Search(PBCallInfo * ci);

protected:
    // member variables
    IPB_Session * m_pSession;
	LPSTR m_sPattern;
	pcre *re;
	bool m_butf8;
	bool m_bGlobal;
	bool m_bCaseSensitive;

	//array of matches
	pbarray matches;

 };

#endif	// !defined(CPBNIREGEX_H)