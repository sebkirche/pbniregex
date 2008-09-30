// My.h : header file for PBNI class
//
// @author : Sebastien Kirche - 2008

#ifndef CPBNIREGEX_H
#define CPBNIREGEX_H

#include <pbext.h>
#include "pcre.h"

#define MAXMATCHES 30
#define OVECCOUNT 60    /* should be a multiple of 3 */

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
		mid_Version,
		mid_Init,
		mid_Test,
		mid_SetUtf,
		mid_Search,
		mid_MatchCount,
		mid_MatchPos,
		mid_MatchLen,
		mid_GroupCount,
		mid_Match,
		mid_Group,
		mid_Replace,
		mid_GroupPos,
		mid_GroupLen,
		mid_SetMulti,
		mid_IsMulti,
		mid_IsUtf,
		NO_MORE_METHODS
	};

public:
	PBXRESULT FastReplace(PBCallInfo * ci);

protected:
 	// methods callable from PowerBuilder
	PBXRESULT Version( PBCallInfo * ci );
	PBXRESULT Hello( PBCallInfo * ci );
	PBXRESULT Initialize(PBCallInfo * ci);
	PBXRESULT Test(PBCallInfo * ci);
	PBXRESULT SetUtf(PBCallInfo * ci);
	PBXRESULT Search(PBCallInfo * ci);
	PBXRESULT MatchCount(PBCallInfo * ci);
	PBXRESULT MatchPos(PBCallInfo * ci);
	PBXRESULT MatchLen(PBCallInfo * ci);
	PBXRESULT GroupCount(PBCallInfo * ci);
	PBXRESULT Match(PBCallInfo * ci);
	PBXRESULT Group(PBCallInfo * ci);
	PBXRESULT Replace(PBCallInfo * ci);
	PBXRESULT GroupPos(PBCallInfo * ci);
	PBXRESULT GroupLen(PBCallInfo * ci);
	PBXRESULT SetMulti(PBCallInfo * ci);
	PBXRESULT IsMulti(PBCallInfo * ci);
	PBXRESULT IsUtf(PBCallInfo * ci);

protected:
    // member variables
    IPB_Session * m_pSession;	// session PB
	LPSTR m_sPattern;			// regexp pattern
	LPSTR m_sData;				// data searched by the regex
	pcre *re;					// compiled regexp
	bool m_butf8;				// option : use utf-8 for regexen / data ?
	bool m_bGlobal;				// option : global search / replace ?
	bool m_bCaseSensitive;		// option : be case-sensitive ?
	bool m_bmultiLine;			// option : multiline

	//space to store the matching info
	int m_matchinfo[MAXMATCHES][OVECCOUNT];	//array of vectors to store matching info
	int m_matchCount;						//number of matches for the current search()
	int m_groupcount[MAXMATCHES];			//number of captured substrings for each match
 };

#endif	// !defined(CPBNIREGEX_H)