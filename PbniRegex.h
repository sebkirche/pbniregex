// My.h : header file for PBNI class
//
// @author : Sebastien Kirche - 2008, 2009, 2011

#ifndef CPBNIREGEX_H
#define CPBNIREGEX_H

#include <pbext.h>
#include "pcre.h"
#include "strconv.h"

#define MAXMATCHES 30
#define MAXGROUPS 20

#ifdef _DEBUG
#define OutputDBSIfDebug(s) OutputDebugString(s)
#define OutputDBSIfDebugA(s) OutputDebugStringA(s)
#else
#define OutputDBSIfDebug(s) 
#define OutputDBSIfDebugA(s) 
#endif

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
		mid_Study,
		mid_getDot,
		mid_setDot,
		mid_getExtended,
		mid_setExtended,
		mid_getUnGreedy,
		mid_setUnGreedy,
		mid_getPattern,
		mid_getLastErr,
#ifdef _DEBUG
		mid_StrTest,
#endif
		NO_MORE_METHODS
	};

public:
	PBXRESULT FastReplace(PBCallInfo * ci);
	PBXRESULT FastReplaceChooseCase(PBCallInfo * ci);
	PBXRESULT FastReplaceCase(PBCallInfo * ci);
	PBXRESULT FastReplaceNoCase(PBCallInfo * ci);

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
	PBXRESULT Study(PBCallInfo * ci);
	PBXRESULT GetDotNL(PBCallInfo * ci);
	PBXRESULT SetDotNL(PBCallInfo * ci);
	PBXRESULT GetExtended(PBCallInfo * ci);
	PBXRESULT SetExtended(PBCallInfo * ci);
	PBXRESULT GetUnGreedy(PBCallInfo * ci);
	PBXRESULT SetUnGreedy(PBCallInfo * ci);
	PBXRESULT GetPattern(PBCallInfo * ci);
	PBXRESULT GetLastErrMsg(PBCallInfo * ci);
#ifdef _DEBUG
	PBXRESULT StrTest(PBCallInfo * ci);
#endif
private:
	void SetLastErrMsg(const char *msg);

protected:
    // member variables
    IPB_Session * m_pSession;	// session PB
	LPSTR m_sPattern;			// regexp pattern
	LPSTR m_sData;				// data searched by the regex
	pcre *re;					// compiled regexp
	pcre_extra *studinfo;		// infos resulting of the pcre_study() call
	long m_maxmatches;			// max match number, can be automagicaly increased during processing
	long m_maxgroups;			// max group number for a match, can be automagicaly increased during processing
	long m_ovecsize;			// size of the vector in number of ints
	bool m_butf8;				// option : use utf-8 for regexen / data ?
	pbboolean m_bGlobal;		// option : global search / replace ?
	pbboolean m_bCaseSensitive;	// option : be case-sensitive ? - maps PCRE_CASELESS == /i option in perl
	pbboolean m_bmultiLine;		// option : multiline			- maps PCRE_MULTILINE == /m option in perl
	pbboolean m_bDotNL;			// option : dot matches Newlines - maps PCRE_DOTALL == /s option in perl
	pbboolean m_bExtended;		// option : use extended regexps - maps PCRE_EXTENDED == /x option in perl
	pbboolean m_bUnGreedy;		// option : ungreedy ?			 - maps PCRE_UNGREEDY

	//space to store the matching info
	int *m_matchinfo;		//buffer for the array of vectors to store matching info
	int *m_replacebuf;		//working buffer for the replace method
	int m_matchCount;		//number of matches for the current search()
	int *m_groupcount;		//number of captured substrings for each match

	HANDLE hHeap;			//private heap handle	
	char *m_lastErr;		//last error message
 };

#endif	// !defined(CPBNIREGEX_H)