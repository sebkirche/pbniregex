// PbniRegex.cpp : PBNI class
//
// @author : Sebastien Kirche - 2008, 2009, 2011, 2013

#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_WARNINGS
#define PCRE_STATIC 1

#include <stdio.h>
#include <string>
#ifdef UNICODE
	#include <mbstring.h>
#endif
#include <assert.h>
#include "main.h"
#include "PbniRegex.h"
#include "strconv.h"
#include "pcre.h"
#include "pcrecpp.h"

#define PCRE_VERSION_PREFIX "PCRE v."
#ifdef _DEBUG
#define	VERSION_STR	STR(" Debug version ") STR(PBX_VERSION) STR(" - ") STR(__DATE__) STR(" ") STR(__TIME__)
#else
#define	VERSION_STR	STR(" Release version ") STR(PBX_VERSION) STR(" - ") STR(__DATE__) STR(" ") STR(__TIME__)
#endif

//this is a buffer used for building OutputDebugString messages
//TODO: improve the hardcoded allocation
char dbgMsg[2048];

// Default constructor for PbniRegex
PbniRegex::PbniRegex()
{
}

// PbniRegx constructor in Session context
PbniRegex::PbniRegex( IPB_Session * pSession )
:m_pSession( pSession )
{
	OutputDBSIfDebug(STR("PbniRegex::Constructor\n"));

	m_bGlobal = false;
	m_bCaseSensitive = false;
	m_bMultiLine = false;
	m_bDotNL = false;
	m_bExtended = false;
	m_bUnGreedy = false;
	m_bDuplicates = false;
	m_sPattern = NULL;
	m_sData = NULL;
	m_re = NULL;
	m_studinfo = NULL;
	m_jitted = false;
	m_matchCount = -1;
	m_maxmatches = MAXMATCHES;
	m_maxgroups = MAXGROUPS;
	/* the size of the vector for n capturing groups is 
	   n + 1 matching infos (+ 1 for the match itself)
	   the result is 2 ints per info and the lib needs 1 more third as working area */
	m_ovecitems = (m_maxgroups + 1) * 3;
	m_ovecbytes = m_ovecitems * sizeof(int);
	m_hHeap = HeapCreate(HEAP_NO_SERIALIZE, 1024 * 64, 0);
	m_matchinfo = (int *)HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY,  sizeof(int) * m_ovecitems * m_maxmatches);
	m_replacebuf = (int *)HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, sizeof(int) * m_ovecitems);
	m_groupcount = (int *)HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, sizeof(int) * m_maxmatches);
	m_lastErr = (LPSTR)HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, 1);
	m_lastErr[0] = '\0';
	m_nameTable = NULL;
	m_nameEntrySize = 0;
	m_nameEntriesCount = 0;
#ifdef _DEBUG
	char *curLocale = setlocale(LC_ALL, NULL);
	_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex: current locale is `%s`\n", curLocale);
	OutputDebugStringA(dbgMsg);
#endif
}

PbniRegex::~PbniRegex(){
	OutputDebugString(STR("PbniRegex::Destructor\n"));

	if(m_sPattern)
		free(m_sPattern);
	if(m_sData)
		free(m_sData);
	if(m_re)
		pcre_free(m_re);
	ClearStudyInfo();
	if(m_matchinfo)
		HeapFree(m_hHeap, 0, m_matchinfo);
	if(m_replacebuf)
		HeapFree(m_hHeap, 0, m_replacebuf);
	if(m_groupcount)
		HeapFree(m_hHeap, 0, m_groupcount);
	if (m_lastErr)
		HeapFree(m_hHeap, 0, m_lastErr);
	if(m_hHeap)
		HeapDestroy(m_hHeap);
}

// Utility function to clear the study information block returned by pcre_study()
void PbniRegex::ClearStudyInfo(void)
{
	if(m_studinfo && m_jitted)
		pcre_free_study(m_studinfo);
	else if (m_studinfo)
		pcre_free(m_studinfo);
	m_studinfo = NULL;
}

// method called by PowerBuilder to invoke PBNI class methods
PBXRESULT PbniRegex::Invoke(IPB_Session * session, pbobject obj, pbmethodID mid, PBCallInfo * ci){

   PBXRESULT pbxr = PBX_OK;

	switch(mid){
		case mid_PcreVersion:
			pbxr = this->PcreVersion( ci );
			break;
		case mid_GetVersion:
			pbxr = this->Version( ci );
			break;
		case mid_GetVersionFull:
			pbxr = this->VersionFull(ci);
			break;
		case mid_Init:
			pbxr = this->Initialize(ci);
			break;
		case mid_Test:
			pbxr = this->Test(ci);
			break;
		case mid_Search:
			pbxr = this->Search(ci);
			break;
		case mid_MatchCount:
			pbxr = this->MatchCount(ci);
			break;
		case mid_MatchPos:
			pbxr = this->MatchPos(ci);
			break;
		case mid_MatchLen:
			pbxr = this->MatchLen(ci);
			break;
		case mid_GroupCount:
			pbxr = this->GroupCount(ci);
			break;
		case mid_Match:
			pbxr = this->Match(ci);
			break;
		case mid_GroupByIndex:
		case mid_GroupByName:
			pbxr = this->Group(ci);
			break;
		case mid_GroupPosByIndex:
		case mid_GroupPosByName:
			pbxr = this->GroupPos(ci);
			break;
		case mid_GroupLenByIndex:
		case mid_GroupLenByName:
			pbxr = this->GroupLen(ci);
			break;
		case mid_Replace:
			pbxr = this->Replace(ci);
			break;
		case mid_SetMulti:
			pbxr = this->SetMulti(ci);
			break;
		case mid_IsMulti:
			pbxr = this->IsMulti(ci);
			break;
		case mid_Study:
			pbxr = this->Study(ci, false);
			break;
		case mid_JitCompile:
			pbxr = this->Study(ci, true);
			break;
		case mid_GetDot:
			pbxr = this->GetDotNL(ci);
			break;
		case mid_SetDot:
			pbxr = this->SetDotNL(ci);
			break;
		case mid_GetExtended:
			pbxr = this->GetExtended(ci);
			break;
		case mid_SetExtended:
			pbxr = this->SetExtended(ci);
			break;
		case mid_GetUnGreedy:
			pbxr = this->GetUnGreedy(ci);
			break;
		case mid_SetUnGreedy:
			pbxr = this->SetUnGreedy(ci);
			break;
		case mid_GetDup:
			pbxr = this->GetDuplicates(ci);
			break;
		case mid_SetDup:
			pbxr = this->SetDuplicates(ci);
			break;
		case mid_GetPattern:
			pbxr = this->GetPattern(ci);
			break;
		case mid_GetLastErr:
			pbxr = this->GetLastErrMsg(ci);
			break;
#ifdef _DEBUG
		case mid_StrTest:
			pbxr = this->StrTest(ci);
			break;
#endif
		default:
			pbxr = PBX_E_INVOKE_METHOD_AMBIGUOUS;
	}
	return pbxr;
}

void PbniRegex::Destroy() 
{
   delete this;
}

/*====================== Methods exposed to PowerBuilder ==========================*/

// Return the PCRE lib version
PBXRESULT PbniRegex::PcreVersion( PBCallInfo * ci ){

	size_t versLen;
	char *verStr;
	PBXRESULT	pbxr = PBX_OK;

	versLen = strlen(PCRE_VERSION_PREFIX) + strlen(pcre_version()) + 1;
	verStr = (LPSTR)malloc(versLen);
	strcpy(verStr, PCRE_VERSION_PREFIX);
	strcat(verStr, pcre_version());
#if defined (PBVER) && (PBVER < 100)
	ci->returnValue->SetString(verStr);
#else
	LPCWSTR wver = AnsiToWC(verStr);
	ci->returnValue->SetString(wver);
	free((void*)wver);
#endif
	free((void*)verStr);
	return pbxr;
}

// Return the PbniRegex short version (e.g. 1.2.3)
PBXRESULT PbniRegex::Version( PBCallInfo * ci ){

	PBXRESULT	pbxr = PBX_OK;

	// return value
#if defined (PBVER) && (PBVER < 100)
	ci->returnValue->SetString(PBX_VERSION);
#else
	LPCWSTR wver = AnsiToWC(PBX_VERSION);
	ci->returnValue->SetString(wver);
	free((void*)wver);
#endif
	return pbxr;
}

// Return the PbniRegex long version, including debug version and time stamp
PBXRESULT PbniRegex::VersionFull( PBCallInfo * ci ){

	PBXRESULT	pbxr = PBX_OK;

	// return value
//#if defined (PBVER) && (PBVER < 100)
//	LPCSTR aver = WCToAnsi(VERSION_STR);
//	ci->returnValue->SetString(aver);
//	free((void*)aver);
//#else
	ci->returnValue->SetString(VERSION_STR);
//#endif
	return pbxr;
}

// Initialize a regexp from a pattern
// If we want to use the jit compiler, we need also to call Study()
PBXRESULT PbniRegex::Initialize(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;
	//pcre *re;
	const char *error;
	int erroffset;
	
	m_Opts = 0;
	m_nameTable = NULL;
	m_nameEntrySize = 0;
	m_nameEntriesCount = 0;

	//if we recompile a new pattern over an old one, clear the memory
	ClearStudyInfo();

	if(ci->pArgs->GetCount() != 3)
		return PBX_E_INVOKE_WRONG_NUM_ARGS;

	if(ci->pArgs->GetAt(0)->IsNull())
		ci->returnValue->SetToNull(); //if pattern is null, return null.
	else
	{
		pbstring arg_pattern = ci->pArgs->GetAt(0)->GetString();
		LPCWSTR pattern_ucs2;
#if defined (PBVER) && (PBVER < 100)
		/* PB >= 10 uses natively utf-16le, while I choosed to process string in PCRE in utf-8
		   if we received the pattern from PB9, we use utf-16le as pivot encoding
		*/
		pattern_ucs2 = AnsiToWC(m_pSession->GetString(arg_pattern));
#else
		pattern_ucs2 = m_pSession->GetString(arg_pattern);
#endif
		m_bGlobal = ci->pArgs->GetAt(1)->GetBool();
		m_bCaseSensitive = ci->pArgs->GetAt(2)->GetBool();

		if (m_sPattern)
			free(m_sPattern);

		m_sPattern = WCToUtf8(pattern_ucs2);

#if defined (PBVER) && (PBVER < 100)
		free((void*)pattern_ucs2);
#else
		CommonReleaseString(m_pSession, pattern_ucs2);
#endif
		m_Opts += PCRE_UTF8;

		m_matchCount = 0;
		if(!m_bCaseSensitive)
			m_Opts += PCRE_CASELESS;
		if(m_bMultiLine)
			m_Opts += PCRE_MULTILINE;
		if(m_bDotNL)
			m_Opts += PCRE_DOTALL;
		if(m_bExtended)
			m_Opts += PCRE_EXTENDED;
		if(m_bUnGreedy)
			m_Opts += PCRE_UNGREEDY;
		if(m_bDuplicates)
			m_Opts += PCRE_DUPNAMES;

		m_re = pcre_compile(
				m_sPattern,           /* pattern */
				m_Opts,               /* options (d�faut = 0)*/
				&error,               /* for error message */
				&erroffset,           /* for error offset */
				NULL);                /* use default character tables */
		if (m_re == NULL){
			if (error){
				_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
				OutputDebugStringA(dbgMsg);
				SetLastErrMsg(dbgMsg);
			}
			ci->returnValue->SetBool(false);
		}
		else{
			ci->returnValue->SetBool(true);
		}
		if(m_re){
			int newmaxgroups;
			if (pcre_fullinfo(m_re, NULL, PCRE_INFO_CAPTURECOUNT, &newmaxgroups) == 0){
				if (newmaxgroups > m_maxgroups){
#ifdef _DEBUG
					_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex :: needs to reallocate bigger block for group matches (maxgroup was %d, increase to %d)\n", m_maxgroups, newmaxgroups);
					OutputDebugStringA(dbgMsg);
#endif
					m_maxgroups = newmaxgroups;
					m_ovecitems = (m_maxgroups + 1) * 3;
					m_ovecbytes = m_ovecitems * sizeof(int);
					m_matchinfo = (int *)HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_matchinfo, m_maxmatches * m_ovecbytes);
					if(!m_matchinfo){
						long err = GetLastError();
						_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex :: initialize failed on memory reallocation for matches\n");
						OutputDebugStringA(dbgMsg);
						SetLastErrMsg(dbgMsg);
						ci->returnValue->SetBool(false);
						pbxr = PBX_E_OUTOF_MEMORY;
					}
					m_replacebuf = (int *)HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_replacebuf, m_ovecbytes);
					if(!m_replacebuf){
						long err = GetLastError();
						_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex :: initialize failed on memory reallocation for replace\n");
						OutputDebugStringA(dbgMsg);
						SetLastErrMsg(dbgMsg);
						ci->returnValue->SetBool(false);
						pbxr = PBX_E_OUTOF_MEMORY;
					}
					m_groupcount = (int *)HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_groupcount, sizeof(int) * m_maxmatches);
					if(!m_groupcount){
						long err = GetLastError();
						_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex :: initialize failed on memory reallocation for groups count\n");
						OutputDebugStringA(dbgMsg);
						SetLastErrMsg(dbgMsg);
						ci->returnValue->SetBool(false);
						pbxr = PBX_E_OUTOF_MEMORY;
					}
				}//mem reallocation needed


				//update the duplicate option
				int optionJ, ret;
				m_bDuplicates = (m_Opts & PCRE_DUPNAMES 
								||
								(((ret = pcre_fullinfo(m_re, m_studinfo, PCRE_INFO_JCHANGED, &optionJ)) == 0) && (optionJ == 1)));
			} else { //pcre-fullinfo was successful
				ci->returnValue->SetBool(false);
				_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex :: pcre_fullinfo() failed...\n");
				OutputDebugStringA(dbgMsg);
				SetLastErrMsg(dbgMsg);
			}//pcre-fullinfo was successful
		}//pcre_compile was successful

	}
	return pbxr;
}

/* Analyze the regexp and find out if extra info can be extracted to speed up repetitive calls to it 
   from PCRE documentation (7.3) :
   At present, studying a pattern is useful only for non-anchored patterns that do not have a single fixed starting character.
   A bitmap of possible starting bytes is created. 
*/
PBXRESULT PbniRegex::Study(PBCallInfo *ci, bool bUseJit){

	const char *error;
	PBXRESULT pbxr = PBX_OK;
	int studyOptions = 0;
	
	//clear the memory in case where Study was already called and resulted in allocated data
	ClearStudyInfo();

	if (bUseJit){
		studyOptions += PCRE_STUDY_JIT_COMPILE;
		m_jitted = true;
	}
	
	m_studinfo = pcre_study(m_re, studyOptions, &error);
	if (error){
	  _snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PCRE study failed with message '%s'\n", error);
	  OutputDebugStringA(dbgMsg);
	  SetLastErrMsg(dbgMsg);
	}
	if (m_studinfo == NULL){
		//if the study did not found useful data
		ci->returnValue->SetBool(false);
	}else{
		//if the study was successful
		ci->returnValue->SetBool(true);
	}
	return pbxr;
}

/* Tell is the given string matches the regexp pattern */
PBXRESULT PbniRegex::Test( PBCallInfo * ci ){

	PBXRESULT	pbxr = PBX_OK;
	int rc, testLen;

	// check arguments
	if ( ci->pArgs->GetAt(0)->IsNull() || !m_re){
		// if any of the passed arguments is null, return the null value
		ci->returnValue->SetBool(false);
	}
	else{
		pbstring pbtest = ci->pArgs->GetAt(0)->GetString();
		LPCWSTR testStr;
#if defined (PBVER) && (PBVER < 100)
		testStr = AnsiToWC(m_pSession->GetString(pbtest));
#else
		testStr = m_pSession->GetString(pbtest);
#endif
		LPSTR testStr_utf8 = WCToUtf8(testStr, &testLen);

		rc = pcre_exec(
		  m_re,                 /* the compiled pattern */
		  m_studinfo,           /* extra data if we studied the pattern */
		  testStr_utf8,         /* the subject string */
		  testLen-1,			/* the length of the subject */
		  0,                    /* start at offset 0 in the subject */
		  0,                    /* default options */
		  //TODO the m_matchinfo can be too short for all matching info
		  m_matchinfo,          /* output vector for substring information */
		  m_ovecitems);			/* number of elements in the output vector */
		if(rc < 0){
			switch(rc){
				case PCRE_ERROR_NOMATCH:
					//normal : no match
					break;
				default:
					//other error case
					break;
			}
			ci->returnValue->SetBool(false);
		}
		else
			ci->returnValue->SetBool(true);

		free(testStr_utf8);
#if defined (PBVER) && (PBVER < 100)
		free((void*)testStr);
#else
		CommonReleaseString(m_pSession, testStr);
#endif
	}
	return pbxr;
}

// Setter for the "dot match newline char" setting
PBXRESULT PbniRegex::SetDotNL(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	m_bDotNL = ci->pArgs->GetAt(0)->GetBool();
	return pbxr;
}

// Setter for the "extended regexp" setting
PBXRESULT PbniRegex::SetExtended(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	m_bExtended = ci->pArgs->GetAt(0)->GetBool();
	return pbxr;
}

// Setter for the "multiline regexp" setting
PBXRESULT PbniRegex::SetMulti(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	m_bMultiLine = ci->pArgs->GetAt(0)->GetBool();
	return pbxr;
}

// Setter for the greediness setting
PBXRESULT PbniRegex::SetUnGreedy(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	m_bUnGreedy = ci->pArgs->GetAt(0)->GetBool();
	return pbxr;
}

// Setter for the "allow name duplicates" setting
PBXRESULT PbniRegex::SetDuplicates(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	m_bDuplicates = ci->pArgs->GetAt(0)->GetBool();
	return pbxr;
}

// Execute the regexp on the given string
// Returns the number of matches or 0
//
// we are calling pcre_exec() in loop to fill 
// - m_matchinfo : an array of matches
// - m_groupcount : an array of number of groups for each match
//
// TODO: we could implement a lazy loading instead of precalculating each match info
//
PBXRESULT PbniRegex::Search(PBCallInfo *ci){

	int nmatch = 0;
	int startoffset = 0;
	int res;
	int searchLen;
	PBXRESULT pbxr = PBX_OK;

	if(ci->pArgs->GetAt(0)->IsNull()){
		// if any of the passed arguments is null, return the null value
		ci->returnValue->SetToNull();
	}
	else {
		pbstring pbsearch = ci->pArgs->GetAt(0)->GetString();
		LPCWSTR searchStr;

		if(m_sData)
			free(m_sData);

#if defined (PBVER) && (PBVER < 100)
		searchStr = AnsiToWC(m_pSession->GetString(pbsearch));
#else
		searchStr = m_pSession->GetString(pbsearch);
#endif
		m_sData = WCToUtf8(searchStr, &searchLen);
#if defined (PBVER) && (PBVER < 100)
		free((void *)searchStr);
#else
		CommonReleaseString(m_pSession, searchStr);
#endif

		do {
			if (nmatch >= m_maxmatches){
				//if we have already filled the vector to the maximum number of matches, reallocate some more space
#ifdef _DEBUG
				_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex :: needs more memory to store matches (max matches was %d)\n", m_maxmatches);
				OutputDebugStringA(dbgMsg);
#endif
				//lets say that we grow the buffer by 50%
				m_maxmatches += m_maxmatches >> 2;
				//m_maxmatches *= 1.5;
				m_maxmatches++; //keep ?
#ifdef _DEBUG
				_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex :: new max matches will be %d\n", m_maxmatches);
				OutputDebugStringA(dbgMsg);
#endif
				//TODO: should handle a try/catch if the mem alloc fails ?
				m_matchinfo = (int *)HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_matchinfo, m_maxmatches * m_ovecbytes);
				if(!m_matchinfo){
					long err = GetLastError();
					_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex :: search failed on memory reallocation for matches\n");
					OutputDebugStringA(dbgMsg);
					SetLastErrMsg(dbgMsg);
					ci->returnValue->SetLong(-1);
					pbxr = PBX_E_OUTOF_MEMORY;
				}
				m_groupcount = (int *)HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_groupcount, sizeof(int) * m_maxmatches);
				if(!m_groupcount){
					long err = GetLastError();
					_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex :: initialize failed on memory reallocation for groups count\n");
					OutputDebugStringA(dbgMsg);
					SetLastErrMsg(dbgMsg);
					ci->returnValue->SetLong(-1);
					pbxr = PBX_E_OUTOF_MEMORY;
				}
			}
			assert(m_ovecitems % 3 == 0);
			res = pcre_exec(m_re, m_studinfo, m_sData, (int)searchLen-1, startoffset, 0, m_matchinfo + nmatch * m_ovecitems, m_ovecitems);
			if (res > 0) {
				m_groupcount[nmatch] = res - 1;
				startoffset = m_matchinfo[(nmatch * m_ovecitems) + 1];
				nmatch++;
				// the match might have a null length : the start offset = the next offset, so we stop there
				if(startoffset == m_matchinfo[((nmatch - 1) * m_ovecitems)])
					break;
				// do not perform another search if we are at the end of the string
				if (startoffset >= (searchLen - 1))
					break;
			} else {
				//res = 0 if not enough space in the vector for all groups - this should not happen
				dbgMsg[0] = '\0';
				switch(res){
				case PCRE_ERROR_NOMATCH:
					break;
				case PCRE_ERROR_NULL:
					_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "pcre_exec failed due to null strings or vector\n");
					break;
				case PCRE_ERROR_BADOPTION:
					_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "pcre_exec failed due to bad option\n");
					break;
				case PCRE_ERROR_NOMEMORY:
					_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "pcre_exec failed due to bad option\n");
					break;
				}
				if(dbgMsg){
					OutputDebugStringA(dbgMsg);
					SetLastErrMsg(dbgMsg);
				}
				break;
			}

		//until there is no match left OR if we did not set the global attribute for a single search
		}while (m_bGlobal && nmatch > 0); //negative res for errors or no match
		
		m_matchCount = nmatch;
		if (nmatch)
			ci->returnValue->SetLong(nmatch);
		else
			ci->returnValue->SetLong(-1);
	}

	return pbxr;
}

// Return the number of matches from the last Search()
PBXRESULT PbniRegex::MatchCount(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	ci->returnValue->SetLong(m_matchCount);
	return pbxr;
}

// Getter for the multiline setting
PBXRESULT PbniRegex::IsMulti(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	ci->returnValue->SetBool(m_bMultiLine);
	return pbxr;
}

// Setter for the "dot match newline char" setting
PBXRESULT PbniRegex::GetDotNL(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	ci->returnValue->SetBool(m_bDotNL);
	return pbxr;
}

// Getter for the "extended regexp" setting
PBXRESULT PbniRegex::GetExtended(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	ci->returnValue->SetBool(m_bExtended);
	return pbxr;
}

// Getter for the greediness setting
PBXRESULT PbniRegex::GetUnGreedy(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	ci->returnValue->SetBool(m_bUnGreedy);
	return pbxr;
}

// Getter for the "allow name duplicates" setting
PBXRESULT PbniRegex::GetDuplicates(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	ci->returnValue->SetBool(m_bDuplicates);
	return pbxr;
}

// Return the position of the match from the last Search()
PBXRESULT PbniRegex::MatchPos(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	long index = ci->pArgs->GetAt(0)->GetLong() - 1; //in PB the index starts at 1
	if(m_re != NULL && index >= 0 && index < m_matchCount)
		ci->returnValue->SetLong(strnlen_utf8((const unsigned char*)m_sData, m_matchinfo[index * m_ovecitems + 0]) + 1);
	else
		ci->returnValue->SetLong(-1);
	return pbxr;
}

// Return the length of the match from the last Search()
PBXRESULT PbniRegex::MatchLen(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	long index = ci->pArgs->GetAt(0)->GetLong() - 1; //in PB the index starts at 1
	if(m_re != NULL && index >= 0 && index <= m_matchCount)
		ci->returnValue->SetLong(strnlen_utf8((const unsigned char*)m_sData + m_matchinfo[index * m_ovecitems + 0], m_matchinfo[index * m_ovecitems + 1] - m_matchinfo[index * m_ovecitems + 0]));
	else
		ci->returnValue->SetLong(-1);
	return pbxr;
}

// Return the position of the given group from the last Search()
PBXRESULT PbniRegex::GroupPos(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;
	long matchindex, groupindex;

	if(ci->pArgs->GetCount() != 2)
		return PBX_E_INVOKE_WRONG_NUM_ARGS;

	matchindex = ci->pArgs->GetAt(0)->GetLong() - 1; // for PB, first match is 1

	if(m_re != NULL && matchindex >= 0 && matchindex <= m_matchCount){
		//by index or by name ?
		if(ci->pArgs->GetAt(1)->GetType() == pbvalue_long)
			groupindex = ci->pArgs->GetAt(1)->GetLong(); //the group 0 is the whole match
		else
			groupindex = GetGroupIndex(matchindex, ci->pArgs->GetAt(1)->GetString());

		if(groupindex == PCRE_ERROR_NOSUBSTRING)
			return PBX_E_INVALID_ARGUMENT;

		if(groupindex >= 0 && groupindex <= m_groupcount[matchindex])
			ci->returnValue->SetLong(strnlen_utf8((const unsigned char*)m_sData, m_matchinfo[matchindex * m_ovecitems + 2 * groupindex]) + 1);
		else
			ci->returnValue->SetLong(-1);
	}
	else
		ci->returnValue->SetLong(-1);
	return pbxr;
}

// Return the length of the given group from the last Search()
PBXRESULT PbniRegex::GroupLen(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;
	long matchindex, groupindex;

	if(ci->pArgs->GetCount() != 2)
		return PBX_E_INVOKE_WRONG_NUM_ARGS;

	matchindex = ci->pArgs->GetAt(0)->GetLong() - 1; // for PB, first match is 1

	if(m_re != NULL && matchindex >= 0 && matchindex <= m_matchCount){
		//by index or by name ?
		if(ci->pArgs->GetAt(1)->GetType() == pbvalue_long)
			groupindex = ci->pArgs->GetAt(1)->GetLong(); //the group 0 is the whole match
		else
			groupindex = GetGroupIndex(matchindex, ci->pArgs->GetAt(1)->GetString());

		if(groupindex == PCRE_ERROR_NOSUBSTRING)
			return PBX_E_INVALID_ARGUMENT;

		if(groupindex >= 0 && groupindex <= m_groupcount[matchindex])
			ci->returnValue->SetLong(strnlen_utf8((const unsigned char*)m_sData + m_matchinfo[matchindex * m_ovecitems + 2 * groupindex], m_matchinfo[matchindex * m_ovecitems + 2 * groupindex + 1] - m_matchinfo[matchindex * m_ovecitems + 2 * groupindex]));
		else
			ci->returnValue->SetLong(-1);
	}
	else
		ci->returnValue->SetLong(-1);
	return pbxr;
}

// Return the number of groups in the pattern
PBXRESULT PbniRegex::GroupCount(PBCallInfo *ci){
	PBXRESULT pbxr = PBX_OK;

	long index = ci->pArgs->GetAt(0)->GetLong() - 1; //in PB the index starts at 1

	if(m_re != NULL && index >= 0 && index <= m_matchCount)
			ci->returnValue->SetLong(m_groupcount[index]);
	else
		ci->returnValue->SetLong(-1);
	return pbxr;
}

// Return a match after a Search()
PBXRESULT PbniRegex::Match(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	int matchLen;
	long index = ci->pArgs->GetAt(0)->GetLong() - 1; //in PB the index starts at 1

	if(m_re != NULL && index >= 0 && index <= m_matchCount){
		//extract the match from the data
		matchLen = m_matchinfo[index * m_ovecitems + 1] - m_matchinfo[index * m_ovecitems + 0] + 1;
		LPSTR match = (LPSTR)malloc(matchLen + 1);
		lstrcpynA(match, (LPCSTR)(m_sData + m_matchinfo[index * m_ovecitems + 0]), matchLen);
		LPWSTR wstr = Utf8ToWC(match);

		// return value
#if defined (PBVER) && (PBVER < 100)
		LPCSTR ansiRet = WCToAnsi(wstr);
		ci->returnValue->SetString(ansiRet);
		free((void*)ansiRet);
#else
		ci->returnValue->SetString(wstr);
#endif
		free(wstr);
		free(match);
	}
	else
		ci->returnValue->SetToNull();
	return pbxr;
}

// Return the pattern that was compiled
PBXRESULT PbniRegex::GetPattern(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;
	
	if (m_sPattern) {
		LPWSTR wstr = Utf8ToWC(m_sPattern);

#if defined (PBVER) && (PBVER < 100)
		LPCSTR ansiStr = WCToAnsi(wstr);
		ci->returnValue->SetString(ansiStr);
		free((void*)ansiStr);
#else
		ci->returnValue->SetString(wstr);
#endif
		free(wstr);
	} 
	else
		ci->returnValue->SetToNull();

	return pbxr;
}

// Return the last error message (or an empty string)
PBXRESULT PbniRegex::GetLastErrMsg(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;

	if (m_lastErr){
		int errLen = mbstowcs(NULL, m_lastErr, strlen(m_lastErr)+1);
		LPWSTR werr = (LPWSTR)malloc((errLen+1) * sizeof(wchar_t));
		mbstowcs(werr, m_lastErr, strlen(m_lastErr)+1);

		ci->returnValue->SetString((LPCTSTR)werr);
		free(werr);
	}
	else
		ci->returnValue->SetString(STR(""));

	return pbxr;
}

// (private) Setter for the error message
void PbniRegex::SetLastErrMsg(const char *msg){

	m_lastErr = (LPSTR)HeapReAlloc(m_hHeap, HEAP_ZERO_MEMORY, m_lastErr, strlen(msg) + 1);
	if (m_lastErr)
		strcpy(m_lastErr, msg);
}

// (private) read named groups data
void PbniRegex::GetNamedGroupsInfos(void){
	if (!m_nameTable){
		//Lazy initialization
		pcre_fullinfo(m_re, m_studinfo, PCRE_INFO_NAMECOUNT, &m_nameEntriesCount);
		if (!m_nameEntriesCount)
			return;
		pcre_fullinfo(m_re, m_studinfo, PCRE_INFO_NAMEENTRYSIZE, &m_nameEntrySize);
		pcre_fullinfo(m_re, m_studinfo, PCRE_INFO_NAMETABLE, &m_nameTable);
	}
}

// Return the group index of a named group
//
// There is 2 cases :
// - duplicates are NOT allowed (you did not called setnameduplicates(true) before initialize() or not used (?J) in the regex pattern)
//   -> you can easilly call pcre_get_stringnumber(name) to get the group number then access the group
// - duplicates ARE allowed
//   -> pcre_get_stringnumber() is not deterministic about the group index it will return
//   -> you need to pcre_get_stringtable_entries() to get the portion of the names table
//      then iterate to see if a group matched something
int PbniRegex::GetGroupIndex(int matchIndex, pbstring pbgroupname){
	
	const char *groupName;
	int ret;
	char *first, *last;	//defined as char* by pcre.h, but actually pointer to entry = int + stringz
	int nameCount, groupIndex = -1;
	char *entryPtr;

#if defined (PBVER) && (PBVER < 100)
	groupName = m_pSession->GetString(pbgroupname);
#else
	LPCWSTR wcGroupName = m_pSession->GetString(pbgroupname);
	groupName = WCToAnsi(wcGroupName);
#endif

	GetNamedGroupsInfos();

	if (m_bDuplicates){
		//duplicates were allowed, we will retrieve the first group with that name that matched something

		//get the entries corresponding to our named group
		//ret = size of each entry in name-to-number table
		if (pcre_get_stringtable_entries(m_re, groupName, &first, &last) < 1)
			goto skip; //not found or no other error
			
		//get the first non-null match if any
		for (entryPtr = first; entryPtr <= last; entryPtr += m_nameEntrySize){
			groupIndex = (*(unsigned char*)entryPtr << 8) | *((unsigned char*)entryPtr+1); //rebuild little-endian short from big-endian data
			if ((m_groupcount[matchIndex] >= groupIndex)
				&& 
				(m_matchinfo[matchIndex * m_ovecitems + 2*groupIndex]) != -1)
				break;
		}

	} else {
		//no duplicate names allowed => we can use pcre_get_stringnumber() directly
		groupIndex = pcre_get_stringnumber(m_re, groupName);
	}

	skip:
	if(groupIndex == PCRE_ERROR_NOSUBSTRING){
		_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PCRE error: no group named `%s` in pattern.\n", groupName);
		OutputDebugStringA(dbgMsg);
		SetLastErrMsg(dbgMsg);
#if defined (PBVER) && (PBVER > 90)
		CommonReleaseString(m_pSession, wcGroupName);
#endif
	}

	return groupIndex;
}

// Return the substring matched by a group
// it can be an empty string
PBXRESULT PbniRegex::Group(PBCallInfo *ci){

	PBXRESULT pbxr = PBX_OK;
	int groupLen;
	long groupindex;

	if(ci->pArgs->GetCount() != 2)
		return PBX_E_INVOKE_WRONG_NUM_ARGS;

	long matchindex = ci->pArgs->GetAt(0)->GetLong() - 1; //in PB the index starts at 1

	if(m_re != NULL && matchindex >= 0 && matchindex <= m_matchCount){
		
		//by index or by name ?
		if(ci->pArgs->GetAt(1)->GetType() == pbvalue_long)
			groupindex = ci->pArgs->GetAt(1)->GetLong(); //the group 0 is the whole match
		else
			groupindex = GetGroupIndex(matchindex, ci->pArgs->GetAt(1)->GetString());

		if(groupindex == PCRE_ERROR_NOSUBSTRING)
			return PBX_E_INVALID_ARGUMENT;

		if(groupindex == -1){
			ci->returnValue->SetToNull(); //group did not match
			return PBX_OK;
		}

		//TODO: replace with pcre_copy_substring() or pcre_get_substring()
		
		if(groupindex >= 0 && groupindex <= m_groupcount[matchindex]){
			if(-1 == m_matchinfo[matchindex * m_ovecitems + 2*groupindex]){
				//the group matched nothing
				ci->returnValue->SetToNull();
			}
			else {
				//extract the match from the data
				groupLen = m_matchinfo[matchindex * m_ovecitems + 2*groupindex + 1] - m_matchinfo[matchindex * m_ovecitems + 2*groupindex] + 1;
				LPSTR group = (LPSTR)malloc(groupLen + 1);
				lstrcpynA(group, (LPCSTR)(m_sData + m_matchinfo[matchindex * m_ovecitems + 2*groupindex]), groupLen);
				LPWSTR wstr = Utf8ToWC(group);

				// return value
#if defined (PBVER) && (PBVER < 100)
				LPCSTR ansiRet = WCToAnsi(wstr);
				ci->returnValue->SetString(ansiRet);
				free((void*)ansiRet);
#else
				ci->returnValue->SetString(wstr);
#endif
				free(wstr);
				free(group);
			}
		}
		else
			ci->returnValue->SetToNull();	//groupindex < 0 or groupindex > groupcount
	}
	else
		ci->returnValue->SetToNull(); //matchindex < 0 or matchindex > matchcount
	return pbxr;
}

#ifdef _DEBUG
unsigned char EURO_CP1252[] = {0x80};
unsigned char EURO_UTF8[] = {0xE2, 0x82, 0xAC};
unsigned char TEST[] = {0x80, 0xE9};

// Debug function : return to PB the string that was passed (to check if encoding is ok)
PBXRESULT PbniRegex::StrTest( PBCallInfo * ci ){

	PBXRESULT	pbxr = PBX_OK;

	if(ci->pArgs->GetCount() != 1)
		return PBX_E_INVOKE_WRONG_NUM_ARGS;

	pbstring pbstr = ci->pArgs->GetAt(0)->GetString();
	LPCTSTR strIn = m_pSession->GetString(pbstr);
	TCHAR strTest[] = STR("the input string was :");
	LPTCH buf;

	if (_tcslen(strIn)){
		buf = new TCHAR[_tcslen(strTest) + _tcslen(strIn) + 1];
		_tcscpy(buf, strTest);
		_tcscat(buf, strIn);
	}else{

#if defined (PBVER) && (PBVER < 100)
		buf = (LPTCH)TEST;
#else
		int bufLen;
		bufLen = _tcslen(strTest) + MultiByteToWideChar(CP_ACP,0,(char*)TEST,sizeof(TEST),0,0) + 1;
		buf = new TCHAR[wcslen(strTest)+sizeof(TEST)+1];
		memset(buf, 0, bufLen*sizeof(TCHAR));
		LPCWSTR valeur = AnsiToWC((char*)TEST);
		wcscpy(buf, strTest);
		wcscat(buf,valeur);
		free((void*)valeur);
#endif
	}
	// return value
	ci->returnValue->SetString( buf );
#if defined (PBVER) && (PBVER >= 100)
	delete(buf);
	CommonReleaseString(m_pSession, strIn);
#endif

	return pbxr;
}
#endif //_DEBUG

// String replacement using the regexp
//
// The replacement string supports the usage of matched groups indexes \1 \2 ... \n (\0 is whole match)
PBXRESULT PbniRegex::Replace(PBCallInfo *ci){

	int nmatch = 0;
	int nbgroups;
	int startoffset = 0;
	int res, matchLen = 0;
	char toexp[128]; //TODO / FIXME get the correct size from max(10, entrysize)
	PBXRESULT pbxr = PBX_OK;

	if(ci->pArgs->GetCount() != 2)
		return PBX_E_INVOKE_WRONG_NUM_ARGS;
	
	if(ci->pArgs->GetAt(0)->IsNull() || ci->pArgs->GetAt(1)->IsNull())
		ci->returnValue->SetToNull();
	else {
		pbstring pbsearch = ci->pArgs->GetAt(0)->GetString();
		LPCWSTR searchWStr;

		//get the utf-16 string
#if defined (PBVER) && (PBVER < 100)
		searchWStr = AnsiToWC(m_pSession->GetString(pbsearch));
#else
		searchWStr = m_pSession->GetString(pbsearch);
#endif
		//search string utf-16 -> utf-8
		LPSTR search_utf8 = WCToUtf8(searchWStr);
#if defined (PBVER) && (PBVER < 100)
		free((void*)searchWStr);
#else
		CommonReleaseString(m_pSession, searchWStr);
#endif
		//replace string utf-16 -> utf-8
		pbstring pbreplace = ci->pArgs->GetAt(1)->GetString();
		LPCWSTR replaceWStr;
#if defined (PBVER) && (PBVER < 100)
		replaceWStr = AnsiToWC(m_pSession->GetString(pbreplace));
#else
		replaceWStr = m_pSession->GetString(pbreplace);
#endif
		int repLen;
		LPSTR rep_utf8 = WCToUtf8(replaceWStr, &repLen);
#if defined (PBVER) && (PBVER < 100)
		free((void*)replaceWStr);
#else
		CommonReleaseString(m_pSession, replaceWStr);
#endif

		using namespace std;
		string working (search_utf8);

		//get the number of capturing patterns
		res = pcre_fullinfo(m_re, m_studinfo, PCRE_INFO_CAPTURECOUNT, &nbgroups); //need to check res ?

		if (!nbgroups) //skip group replacement if no group
			goto nogroup;

		GetNamedGroupsInfos();
		do {
			//crawl the matches and replace by the replacement string
			char *entryPtr = NULL, *tmpEntry;
			res = pcre_exec(m_re, m_studinfo, working.c_str(), strlen(working.c_str()), startoffset, 0, m_replacebuf, m_ovecitems);
			if (res > 0) {
				string rep(rep_utf8);
				unsigned int p=0, k, grplen, backslashcount;
				
				//expansion of substrings
				//-----------------------
				//we generate the group notation \nn (in decrasing order to avoid replacing e.g. \1 when there is also \10)
				//then if the notation is found, we expand it with the corresponding match
				//else (if did not matched) we delete the notation from the replace string
				for (int curgroup = nbgroups; curgroup >= 0; curgroup--){
					bool bTryingGroupName = false;
					int repgroup, testgroup;

					//look for the group by its index
					_snprintf(toexp, sizeof(toexp) - 1, "\\%d", curgroup);

secondTryWithName:
					grplen = strlen(toexp);
					p = 0;
					while ((p = rep.find(toexp, p)) != string::npos){
						//check if escaped backslashes
						backslashcount = rep[p] == '\\' ? 1 : 0;				//count 1 if the pattern begins with \ char
						for (k=p; k > 0 && rep[k-1]=='\\'; k--){	//then 1 more for each consecutive \ until the begining of string
							backslashcount++;
						}
						if ((backslashcount & 1) == (unsigned int)(bTryingGroupName ? 0 : 1)){ //if zero or even number of backslashes it's ok to replace
							
							repgroup = curgroup;
							//either we are replacing the \nn notation and curgroup is ok
							//or we are looking for another named group
							if(m_replacebuf[repgroup*2] == -1 && bTryingGroupName && m_bDuplicates && entryPtr != NULL){
								int cmp;
								for (tmpEntry = entryPtr-m_nameEntrySize/*start with previous entry*/;
										tmpEntry >= m_nameTable ; /*until we reach the first entry*/
										tmpEntry -= m_nameEntrySize /*we search backwards*/){
										cmp = strncmp(tmpEntry+2, entryPtr+2, m_nameEntrySize-2); 
										testgroup = (tmpEntry - m_nameTable) / m_nameEntrySize + 1;
										if ((!cmp && m_replacebuf[testgroup *2] > -1) || cmp){
											repgroup = testgroup;
											break;
										}
									}
							}
							

							if (m_replacebuf[repgroup*2] > -1){ //when a group matches nothing its offset equals -1
								//do NOT map byte offsets into characters offsets : it is ok for replacement
								rep.replace(p, grplen, working.substr(m_replacebuf[repgroup*2], m_replacebuf[repgroup*2 +1] - m_replacebuf[repgroup*2]));
							}
							else { //no match for group notation -> replace with empty string
								rep.erase(p, grplen);
							}
						} else
							p += grplen;
					}

					//the case of named groups : generate the $+{name} notation corresponding to the current group
					if (m_nameTable && !bTryingGroupName){
						//search a name corresponding to the current processed group in the name table
						for (entryPtr = m_nameTable; entryPtr < m_nameTable + m_nameEntriesCount * m_nameEntrySize; entryPtr += m_nameEntrySize){
							if( ((*(unsigned char*)entryPtr << 8) | *((unsigned char*)entryPtr+1)) == curgroup){ //rebuild the big-endian index
								//if that group matched something
								bTryingGroupName = true;
								_snprintf(toexp, sizeof(toexp) - 1, "$+{%s}", entryPtr + 2);
								goto secondTryWithName;
							}
						}
					}

				}
				//replace escaped backslashes
				p=0;
				while ((p = rep.find("\\\\", p)) != string::npos){
					rep.replace(p++, 2, "\\"); //replace and increase pointer, for not eating all successive backslashes
				}

				//replace the match by the replace string
				matchLen = m_replacebuf[1] - m_replacebuf[0];
				repLen = rep.length();
				working.replace(m_replacebuf[0], matchLen, rep);
				startoffset = m_replacebuf[0] + repLen;
				nmatch++;
			}
		//until there is no match left OR if we did not set the global attribute for a single replacement
		//also, if we replaced nothing by nothing, we can stop too
		}while (m_bGlobal && (res > 0) && (matchLen || repLen));
nogroup:

#ifdef _DEBUG
		OutputDebugStringA(working.c_str());
#endif
		LPWSTR wstr = Utf8ToWC(working.c_str());

#if defined (PBVER) && (PBVER < 100)
		LPCSTR ansiStr = WCToAnsi(wstr);
		ci->returnValue->SetString(ansiStr);
		free((void*)ansiStr);
#else
		ci->returnValue->SetString(wstr);
#endif

		free(search_utf8);
		free(rep_utf8);
		free(wstr);
	}
	return pbxr;
}

// Fast string replacement => global function
// by "fast" I mean "faster than using pbscript built-in Pos() and Mid()"
PBXRESULT PbniRegex::FastReplace(PBCallInfo *ci){
	PBXRESULT pbxr = PBX_OK;

	if(ci->pArgs->GetCount() != 3)
		//if parameter count  <> 3 -> error to PB
		return PBX_E_INVOKE_WRONG_NUM_ARGS;
	if(ci->pArgs->GetAt(0)->IsNull() || ci->pArgs->GetAt(1)->IsNull() || ci->pArgs->GetAt(2)->IsNull())
		//if at less 1 param is null, return null
		ci->returnValue->SetToNull();
	else{
		using namespace std; //for std::wstring
		pbstring source = ci->pArgs->GetAt(0)->GetString();
		pbstring pattern = ci->pArgs->GetAt(1)->GetString();

		LPCTSTR s = m_pSession->GetString(source);
		LPCTSTR p = m_pSession->GetString(pattern);
#ifdef _DEBUG
		_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex::FastReplace, source = %ls\n", s);
		OutputDebugStringA(dbgMsg);
		_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex::FastReplace, pattern = %ls\n", p);
		OutputDebugStringA(dbgMsg);
#endif

		//sanity check : if empty pattern string, return source string
		if(_tcslen(p) == 0){
			ci->returnValue->SetPBString(source);
			return pbxr;
		}

		TCHAR* pos = _tcsstr((TCHAR*)s, (TCHAR*)p);
		if(0){
			stdstring sourcew(s);
			stdstring patternw(p);
		}
		//test for one occurence
		if(/*_tcsstr(s, p)*/pos){
		
			OutputDBSIfDebugA("PbniRegex::FastReplace, found at least one occurrence.\n");
			pbstring replace = ci->pArgs->GetAt(2)->GetString();
			//stdstring replacew(m_pSession->GetString(replace));
			TCHAR* rpl = (TCHAR*)m_pSession->GetString(replace);
			TCHAR* trg = _tcsdup(s);
			TCHAR* prev;
			//here is the 'all' of 'replaceall' : replace each occurence
			//unsigned int p = 0, startoffset = 0;
			size_t p_len = _tcslen(p);
			size_t r_len = _tcslen(rpl);

			if(p_len<r_len){
			//si length(rpl) > length(pattern) alors il faut compter combien il y a d'occurance
			// et augmenter le buffer de la chaine de destination de: Occurences x ( length(rpl) - length(pattern) )
				prev=pos;
				unsigned long occ = 1;
				while(pos = _tcsstr(pos + p_len, p)){
					occ++;
				}
				pos = prev;
				trg = (TCHAR*)realloc((void*)trg, sizeof(TCHAR)*(1 + _tcslen(s) + occ*(r_len - p_len) ) );
			}
			prev = (TCHAR*)s;
			trg[0] = (TCHAR)0;
			TCHAR* tmp_trg = trg;
			size_t t_len;		//taille des chunks interm�diaires
			while(pos/*(p = sourcew.find(patternw, startoffset)) != string::npos*/){
#ifdef _DEBUG
//				_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex::FastReplace, sourcew.find(%ls, %d) found offset %d\n", patternw.c_str(), startoffset, p);
//				OutputDebugStringA(dbgMsg);
#endif
				//sourcew.replace(p, patternw.length(), replacew);
				//startoffset = p + replacew.length();
				
				t_len = pos - prev;
				_tcsncat(tmp_trg, prev, t_len);	//copie ce qui est avant le match (depuis le pr�c�dent match || d�but)
				tmp_trg += t_len;
				_tcscat(tmp_trg, rpl);
				tmp_trg += r_len;
				prev = pos + p_len;
				pos = _tcsstr(prev,p);
			}
			_tcscat(tmp_trg, prev);
			//return the resulting string
#ifdef _DEBUG
//			_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "PbniRegex::FastReplace, final string is %ls\n", sourcew.c_str());
//			OutputDebugStringA(dbgMsg);
#endif
			ci->returnValue->SetString(trg/*sourcew.c_str()*/);
			//free(trg);//do not need anymore, but crash immediatelly in Debug and sometime on release
		}
		else {
			//if no occurrence, return the given string
			OutputDBSIfDebugA("PbniRegex::FastReplace, no occurrence.\n");
			ci->returnValue->SetPBString(source);
		}
	}
	return pbxr;
}

// Fast string replacement => global function
// this version can be case sensitive or insensitive
PBXRESULT PbniRegex::FastReplaceCase(PBCallInfo *ci){

	using namespace std; //for std::wstring
	pbstring source = ci->pArgs->GetAt(0)->GetString();
	pbstring pattern = ci->pArgs->GetAt(1)->GetString();
	pbstring replace = ci->pArgs->GetAt(2)->GetString();

	LPCTSTR sourceStr = m_pSession->GetString(source);
	LPCTSTR patternStr = m_pSession->GetString(pattern);
	LPCTSTR replaceStr = m_pSession->GetString(replace);

	stdstring sourcew(sourceStr);
	stdstring patternw(patternStr);
	stdstring replacew(replaceStr);

#if defined (PBVER) && (PBVER > 90)
	CommonReleaseString(m_pSession, sourceStr);
	CommonReleaseString(m_pSession, patternStr);
	CommonReleaseString(m_pSession, replaceStr);
#endif

	//sanity check : if empty pattern string, return source string
	if(patternw.length() == 0){
		ci->returnValue->SetPBString(source);
		return PBX_OK;
	}

	unsigned int p = 0, startoffset = 0;
	//test for one occurence
	if((p = sourcew.find(patternw, startoffset)) != string::npos){
		//here is the 'all' of 'replaceall' : replace each occurence
		do{	
			sourcew.replace(p, patternw.length(), replacew);
			startoffset = p + replacew.length();
		} while((p = sourcew.find(patternw, startoffset)) != string::npos);
		//return the resulting string
		ci->returnValue->SetString(sourcew.c_str());
	}
	else
		//if no occurrence, return the given string
		ci->returnValue->SetPBString(source);

	return PBX_OK;
}

// case insensitive character traits
// inherited copy (preserves case),
// case insensitive comparison, search
struct traits_nocase : std::char_traits<char> {

	static bool eq( const char& c1, const char& c2 ) { return toupper(c1) == toupper(c2) ; }
	static bool lt( const char& c1, const char& c2 ) { return toupper(c1) < toupper(c2) ; }
	static int compare( const char* s1, const char* s2, size_t N )
	{
	  //return strncasecmp( s1, s2, N ) ; // posix
	  return _strnicmp( s1, s2, N ) ; //microsoft
	}
	static const char* find( const char* s, size_t N, const char& a )
	{
	  for( size_t i=0 ; i<N ; ++i )
		if( toupper(s[i]) == toupper(a) ) return s+i ;
	  return 0 ;
	}
	static bool eq_int_type ( const int_type& c1, const int_type& c2 ) { return toupper(c1) == toupper(c2) ; }
};
// string preserves case; comparisons are case insensitive
typedef std::basic_string< char, traits_nocase > string_nocase ;

// make string_nocase work like a std::string
//           with streams using std::char_traits
// std::basic_istream< char, std::char_traits<char> > (std::istream) and
// std::basic_ostream< char, std::char_traits<char> > (std::ostream)
inline std::ostream& operator<< ( std::ostream& stm, const string_nocase& str ){

	return stm << reinterpret_cast<const std::string&>(str); 
}

inline std::istream& operator>> ( std::istream& stm, string_nocase& str ){

	std::string s ; stm >> s ;
	if(stm) str.assign(s.begin(),s.end()) ;
	return stm ;
}

inline std::istream& getline( std::istream& stm, string_nocase& str ){

	std::string s ; std::getline(stm,s) ;
	if(stm) str.assign(s.begin(),s.end()) ;
	return stm ;
}

// case insensitive character traits
// inherited copy (preserves case),
// case insensitive comparison, search
struct traitws_nocase : std::char_traits<TCHAR>{

	static bool eq( const TCHAR& c1, const TCHAR& c2 ) { return toupper(c1) == toupper(c2) ; }
	static bool lt( const TCHAR& c1, const TCHAR& c2 ) { return toupper(c1) < toupper(c2) ; }
	static int compare( const TCHAR* s1, const TCHAR* s2, size_t N )
	{
	  //return strncasecmp( s1, s2, N ) ; // posix
	  return _tcsnicmp( s1, s2, N ) ; //microsoft
	}
	static const TCHAR* find( const TCHAR* s, size_t N, const TCHAR& a )
	{
	  for( size_t i=0 ; i<N ; ++i )
		if( toupper(s[i]) == toupper(a) ) return s+i ;
	  return 0 ;
	}
	static bool eq_int_type ( const int_type& c1, const int_type& c2 ) { return toupper(c1) == toupper(c2) ; }
};
// string preserves case; comparisons are case insensitive
typedef std::basic_string< TCHAR, traitws_nocase > tstring_nocase ;

// make wstring_nocase work like a std::wstring
//           with streams using std::char_traits
// std::basic_istream< char, std::char_traits<char> > (std::istream) and
// std::basic_ostream< char, std::char_traits<char> > (std::ostream)
#if defined (PBVER) && (PBVER < 100)
inline std::ostream& operator<< ( std::ostream& stm, const tstring_nocase& str ){

	return stm << reinterpret_cast<const std::string&>(str); 
}

inline std::istream& operator>> ( std::istream& stm, tstring_nocase& str ){

	std::string s ; stm >> s ;
	if(stm) str.assign(s.begin(),s.end()) ;
	return stm ;
}

inline std::istream& getline( std::istream& stm, tstring_nocase& str ){

	std::string s ; std::getline(stm,s) ;
	if(stm) str.assign(s.begin(),s.end()) ;
	return stm ;
}

#else
inline std::wostream& operator<< ( std::wostream& stm, const tstring_nocase& str ){

	return stm << reinterpret_cast<const std::wstring&>(str); 
}

inline std::wistream& operator>> ( std::wistream& stm, tstring_nocase& str ){

	std::wstring s ; stm >> s ;
	if(stm) str.assign(s.begin(),s.end()) ;
	return stm ;
}

inline std::wistream& getline( std::wistream& stm, tstring_nocase& str ){

	std::wstring s ; std::getline(stm,s) ;
	if(stm) str.assign(s.begin(),s.end()) ;
	return stm ;
}
#endif

PBXRESULT PbniRegex::FastReplaceNoCase(PBCallInfo *ci){

	using namespace std; //for std::wstring
	pbstring source = ci->pArgs->GetAt(0)->GetString();
	pbstring pattern = ci->pArgs->GetAt(1)->GetString();
	pbstring replace = ci->pArgs->GetAt(2)->GetString();

	LPCTSTR sourceStr = m_pSession->GetString(source);
	LPCTSTR patternStr = m_pSession->GetString(pattern);
	LPCTSTR replaceStr = m_pSession->GetString(replace);

	tstring_nocase sourcew(sourceStr);
	tstring_nocase patternw(patternStr);
	tstring_nocase replacew(replaceStr);

#if defined (PBVER) && (PBVER > 90)
	CommonReleaseString(m_pSession, sourceStr);
	CommonReleaseString(m_pSession, patternStr);
	CommonReleaseString(m_pSession, replaceStr);
#endif

	//sanity check : if empty pattern string, return source string
	if(patternw.length() == 0){
		ci->returnValue->SetPBString(source);
		return PBX_OK;
	}

	unsigned int p = 0, startoffset = 0;
	//test for one occurence
	if((p = sourcew.find(patternw, startoffset)) != string_nocase::npos){
		//here is the 'all' of 'replaceall' : replace each occurence
		do{	
			sourcew.replace(p, patternw.length(), replacew);
			startoffset = p + replacew.length();
		} while((p = sourcew.find(patternw, startoffset)) != string_nocase::npos);
		//return the resulting string
		ci->returnValue->SetString(sourcew.c_str());
	}
	else
		//if no occurrence, return the given string
		ci->returnValue->SetPBString(source);

	return PBX_OK;
}

PBXRESULT PbniRegex::FastReplaceChooseCase(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	if(ci->pArgs->GetCount() != 4)
		//if parameter count  <> 4 -> error to PB
		return PBX_E_INVOKE_WRONG_NUM_ARGS;
	if(ci->pArgs->GetAt(0)->IsNull() || ci->pArgs->GetAt(1)->IsNull() || ci->pArgs->GetAt(2)->IsNull())
		//if at less 1 param is null, return null
		ci->returnValue->SetToNull();
	else{
		using namespace std; //for std::wstring
		pbboolean casesensitive = FALSE;

		casesensitive = ci->pArgs->GetAt(3)->GetBool();
		if(casesensitive)
			pbxr = FastReplaceCase(ci);
		else
			pbxr = FastReplaceNoCase(ci);
	}
	return pbxr;
}

