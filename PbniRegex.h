// PbniRegex.h : header file for PBNI class
//
// @author : Sebastien Kirche - 2008, 2009, 2011, 2013

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
	// WARNING : the enum order must match the functions declarations in Main.cpp::PBX_GetDescription()
	enum Function_Entries
	{
		mid_PcreVersion = 0,
		mid_Init,
		mid_Test,
		mid_Search,
		mid_MatchCount,
		mid_MatchPos,
		mid_MatchLen,
		mid_GroupCount,
		mid_Match,
		mid_GroupByIndex,
		mid_GroupByName,
		mid_Replace,
		mid_GroupPosByIndex,
		mid_GroupPosByName,
		mid_GroupLenByIndex,
		mid_GroupLenByName,
		mid_SetMulti,
		mid_IsMulti,
		mid_Study,
		mid_JitCompile,
		mid_GetDot,
		mid_SetDot,
		mid_GetExtended,
		mid_SetExtended,
		mid_GetUnGreedy,
		mid_SetUnGreedy,
		mid_GetPattern,
		mid_GetLastErr,
		mid_GetVersion,
		mid_GetVersionFull,
		mid_SetDup,
		mid_GetDup,
#ifdef _DEBUG
		mid_StrTest,
#endif
		NO_MORE_METHODS
	};

public:
	PBXRESULT FastReplace(PBCallInfo * ci);
	PBXRESULT FastReplaceCase(PBCallInfo * ci);
	PBXRESULT FastReplaceChooseCase(PBCallInfo * ci);
	PBXRESULT FastReplaceNoCase(PBCallInfo * ci);

protected:
 	// methods callable from PowerBuilder
	PBXRESULT GetDotNL(PBCallInfo * ci);
	PBXRESULT GetDuplicates(PBCallInfo * ci);
	PBXRESULT GetExtended(PBCallInfo * ci);
	PBXRESULT GetLastErrMsg(PBCallInfo * ci);
	PBXRESULT GetPattern(PBCallInfo * ci);
	PBXRESULT Version(PBCallInfo * ci);
	PBXRESULT VersionFull(PBCallInfo * ci);
	PBXRESULT GetUnGreedy(PBCallInfo * ci);
	PBXRESULT Group(PBCallInfo * ci);
	PBXRESULT GroupCount(PBCallInfo * ci);
	PBXRESULT GroupLen(PBCallInfo * ci);
	PBXRESULT GroupPos(PBCallInfo * ci);
	PBXRESULT Initialize(PBCallInfo * ci);
	PBXRESULT IsMulti(PBCallInfo * ci);
	PBXRESULT Match(PBCallInfo * ci);
	PBXRESULT MatchCount(PBCallInfo * ci);
	PBXRESULT MatchLen(PBCallInfo * ci);
	PBXRESULT MatchPos(PBCallInfo * ci);
	PBXRESULT Replace(PBCallInfo * ci);
	PBXRESULT Search(PBCallInfo * ci);
	PBXRESULT SetDotNL(PBCallInfo * ci);
	PBXRESULT SetDuplicates(PBCallInfo * ci);
	PBXRESULT SetExtended(PBCallInfo * ci);
	PBXRESULT SetMulti(PBCallInfo * ci);
	PBXRESULT SetUnGreedy(PBCallInfo * ci);
#ifdef _DEBUG
	PBXRESULT StrTest(PBCallInfo * ci);
#endif
	PBXRESULT Study(PBCallInfo * ci, bool bUseJit);
	PBXRESULT Test(PBCallInfo * ci);
	PBXRESULT PcreVersion( PBCallInfo * ci );
private:
	void SetLastErrMsg(const char *msg);
	void ClearStudyInfo(void);
	int GetGroupIndex(int matchIndex, pbstring pbgroupname);
	void GetNamedGroupsInfos(void);

protected:
    // member variables
    IPB_Session * m_pSession;	// session PB
	LPSTR m_sPattern;			// regexp pattern (local copy, utf-8)
	LPSTR m_sData;				// data searched by the regex
	pcre *m_re;					// compiled regexp
	pcre_extra *m_studinfo;		// infos resulting of the pcre_study() call
	bool m_jitted;				// did we invoked the jit in this regexp ?
	long m_maxmatches;			// max match number, can be automagicaly increased during processing
	long m_maxgroups;			// max group number for a match, can be automagicaly increased during processing
	long m_ovecsize;			// size of the vector in number of ints
	int m_Opts;					// compile options given to pcre_compile()
	pbboolean m_bGlobal;		// option : global search / replace ?
	pbboolean m_bCaseSensitive;	// option : be case-sensitive ? - maps PCRE_CASELESS == /i option in perl
	pbboolean m_bMultiLine;		// option : multiline			- maps PCRE_MULTILINE == /m option in perl
	pbboolean m_bDotNL;			// option : dot matches Newlines - maps PCRE_DOTALL == /s option in perl
	pbboolean m_bExtended;		// option : use extended regexps - maps PCRE_EXTENDED == /x option in perl
	pbboolean m_bUnGreedy;		// option : ungreedy ?			 - maps PCRE_UNGREEDY
	pbboolean m_bDuplicates;	// option : allow duplicates ?	 - maps PCRE_DUPNAMES

	//space to store the matching info
	int *m_matchinfo;		//buffer for the array of vectors to store matching info
	int *m_replacebuf;		//working buffer for the replace method
	int m_matchCount;		//number of matches for the current search()
	int *m_groupcount;		//number of captured substrings for each match

	//info of named capturing groups - see pcreapi for details
	char *m_nameTable;		//a table of named groups
	int m_nameEntrySize;	//the max size of the names (=size of the biggest)
	int m_nameEntriesCount;	//the number of items in the table

	HANDLE m_hHeap;			//private heap handle	
	char *m_lastErr;		//last error message
 };

#endif	// !defined(CPBNIREGEX_H)