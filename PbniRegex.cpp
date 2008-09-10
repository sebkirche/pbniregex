// PbniRegex.cpp : PBNI class
#define _CRT_SECURE_NO_DEPRECATE
#define PCRE_STATIC 1

#include <stdio.h>
#include "PbniRegex.h"
#include "pcre.h"
#include "pcrecpp.h"

char dbgMsg[512];

// default constructor
PbniRegex::PbniRegex()
{
}

PbniRegex::PbniRegex( IPB_Session * pSession )
:m_pSession( pSession )
{
	//OutputDebugString(_T("PbniRegex :: Constructor"));
	m_bGlobal = false;
	m_bCaseSensitive = false;
	m_butf8 = false;
	m_sPattern = NULL;
	m_sData = NULL;
	re = NULL;
	m_matchCount = 0;
}

// destructor
PbniRegex::~PbniRegex()
{
	//OutputDebugString(_T("PbniRegex :: Destructor"));
	if(m_sPattern)
		free(m_sPattern);
	if(m_sData)
		free(m_sData);
	if(re)
		pcre_free(re);
}

// method called by PowerBuilder to invoke PBNI class methods
PBXRESULT PbniRegex::Invoke
(
	IPB_Session * session,
	pbobject obj,
	pbmethodID mid,
	PBCallInfo * ci
)
{
   PBXRESULT pbxr = PBX_OK;

	switch ( mid )
	{
		case mid_Version:
			pbxr = this->Version( ci );
			break;
		case mid_Hello:
			pbxr = this->Hello( ci );
			break;
		case mid_Init:
			pbxr = this->Initialize(ci);
			break;
		case mid_Test:
			pbxr = this->Test(ci);
			break;
		case mid_SetUtf:
			pbxr = this->SetUtf(ci);
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
		case mid_Group:
			pbxr = this->Group(ci);
			break;
		case mid_Replace:
			pbxr = this->Replace(ci);
			break;
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

PBXRESULT PbniRegex::Version( PBCallInfo * ci )
{
	const char *verStr;
	PBXRESULT	pbxr = PBX_OK;

	verStr = pcre_version();
	int verLen = mbstowcs(NULL, verStr, strlen(verStr)+1);
	LPWSTR wstr = (LPWSTR)malloc((verLen+1) * sizeof(wchar_t));
	mbstowcs(wstr, verStr, strlen(verStr)+1);

	// return value
	ci->returnValue->SetString(wstr);
	free(wstr);
	return pbxr;
}

PBXRESULT PbniRegex::Hello( PBCallInfo * ci )
{
	PBXRESULT	pbxr = PBX_OK;

	// return value
	ci->returnValue->SetString( _T("Hello from PbniRegex") );
	OutputDebugStringA(pcre_version());
	return pbxr;
}

PBXRESULT PbniRegex::Initialize(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;
	//pcre *re;
	const char *error;
	int erroffset;
	int opts = 0;

	if(ci->pArgs->GetCount() != 3 || ci->pArgs->GetAt(0)->IsNull())
	{
		// if any of the passed arguments is null, return the null value
		//ci->returnValue->SetBool(false);
		return PBX_E_INVOKE_WRONG_NUM_ARGS;
	}
	else
	{
		pbstring test = ci->pArgs->GetAt(0)->GetString();
		LPCTSTR pattern_ucs2 = m_pSession->GetString(test);
		m_bGlobal = ci->pArgs->GetAt(1)->GetBool();
		m_bCaseSensitive = ci->pArgs->GetAt(2)->GetBool();

		if (m_sPattern)
			free(m_sPattern);
		int patternLen = wcstombs(NULL, (LPWSTR)pattern_ucs2, 0) + 2;
		m_sPattern = (LPSTR)malloc(patternLen);
		if (m_butf8){
			WideCharToMultiByte(CP_UTF8,0,pattern_ucs2,-1,m_sPattern,patternLen,NULL,NULL);	
			opts += PCRE_UTF8;
		}
		else
			WideCharToMultiByte(CP_ACP,0,pattern_ucs2,-1,m_sPattern,patternLen,NULL,NULL);	

		m_matchCount = 0;
		if(!m_bCaseSensitive)
			opts += PCRE_CASELESS;

		re = pcre_compile(
				m_sPattern,              /* the pattern */
				opts,                    /* default options */
				&error,               /* for error message */
				&erroffset,           /* for error offset */
				NULL);                /* use default character tables */
		if (re == NULL){
		  sprintf(dbgMsg, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
		  OutputDebugStringA(dbgMsg);
		  ci->returnValue->SetBool(false);
		}
		else{
			ci->returnValue->SetBool(true);
		}
	}
	return pbxr;
}

/* Tell is the given string matches the regexp pattern */
PBXRESULT PbniRegex::Test( PBCallInfo * ci )
{
	PBXRESULT	pbxr = PBX_OK;
	//pcre *re;
	const char *error;
	int erroffset;
	int rc;
	int ovector[OVECCOUNT];

	// check arguments
	if ( ci->pArgs->GetAt(0)->IsNull() || !re)
	{
		// if any of the passed arguments is null, return the null value
		ci->returnValue->SetBool(false);
	}
	else
	{
		pbstring test = ci->pArgs->GetAt(0)->GetString();
		LPCTSTR testString = m_pSession->GetString(test);

		int testLen = wcstombs(NULL, (LPWSTR)testString, 0) + 2;
		LPSTR testStr_utf8 = (LPSTR)malloc(testLen);
		if (m_butf8)
		{
			WideCharToMultiByte(CP_UTF8,0,testString,-1,testStr_utf8,testLen,NULL,NULL);	
		}
		else
			WideCharToMultiByte(CP_ACP,0,testString,-1,testStr_utf8,testLen,NULL,NULL);	
		rc = pcre_exec(
		  re,                   /* the compiled pattern */
		  NULL,                 /* no extra data - we didn't study the pattern */
		  testStr_utf8,              /* the subject string */
		  testLen-2,       /* the length of the subject */
		  0,                    /* start at offset 0 in the subject */
		  0,                    /* default options */
		  ovector,              /* output vector for substring information */
		  OVECCOUNT);           /* number of elements in the output vector */
		if(rc < 0)
		{
			switch(rc)
			{
			case PCRE_ERROR_NOMATCH:
				//normal no match
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

	}
	return pbxr;
}

PBXRESULT PbniRegex::SetUtf(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	m_butf8 = ci->pArgs->GetAt(0)->GetBool();

	return pbxr;
}

PBXRESULT PbniRegex::Search(PBCallInfo *ci)
{
	int nmatch = 0;
	int maxmatch = MAXMATCHES;
	int startoffset = 0;
	int res;
	//int ovector[OVECCOUNT];

	PBXRESULT pbxr = PBX_OK;
	if(ci->pArgs->GetAt(0)->IsNull()){
		// if any of the passed arguments is null, return the null value
		ci->returnValue->SetToNull();
	}
	else {
		pbstring search = ci->pArgs->GetAt(0)->GetString();
		LPCTSTR searchString = m_pSession->GetString(search);

		if(m_sData)
			free(m_sData);
		int searchLen = wcstombs(NULL, (LPWSTR)searchString, 0) + 2;
		/*LPSTR searchString_utf8*/m_sData = (LPSTR)malloc(searchLen);
		if (m_butf8){
			WideCharToMultiByte(CP_UTF8,0,searchString,-1,m_sData,searchLen,NULL,NULL);	
		}
		else
			WideCharToMultiByte(CP_ACP,0,searchString,-1,m_sData,searchLen,NULL,NULL);	

		do {
			res = pcre_exec(re, NULL, m_sData, (int)searchLen-2, startoffset, 0, m_matchinfo[nmatch], OVECCOUNT);
			if (res > 0) {
				m_groupcount[nmatch] = res - 1;
				startoffset = m_matchinfo[nmatch][1];
				nmatch++;
			} else
				break;
		}while (m_bGlobal && nmatch < maxmatch);
		
		m_matchCount = nmatch;
		if (nmatch)
			ci->returnValue->SetLong(nmatch);
		else
			ci->returnValue->SetLong(-1);

		//free(searchString_utf8);
	}

	return pbxr;
}

PBXRESULT PbniRegex::MatchCount(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	ci->returnValue->SetLong(m_matchCount);
	return pbxr;
}

PBXRESULT PbniRegex::MatchPos(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;
	long index = ci->pArgs->GetAt(0)->GetLong() - 1; //in PB the index starts at 1
	if(index >= 0 && index < m_matchCount)
		ci->returnValue->SetLong(m_matchinfo[index][0] + 1);
	else
		ci->returnValue->SetLong(-1);
	return pbxr;
}

PBXRESULT PbniRegex::MatchLen(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;
	long index = ci->pArgs->GetAt(0)->GetLong() - 1; //in PB the index starts at 1

	if(index >= 0 && index <= m_matchCount)
		ci->returnValue->SetLong(m_matchinfo[index][1] - m_matchinfo[index][0]);
	else
		ci->returnValue->SetLong(-1);
	return pbxr;
}

PBXRESULT PbniRegex::GroupCount(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;
	long index = ci->pArgs->GetAt(0)->GetLong() - 1; //in PB the index starts at 1

	if(index >= 0 && index <= m_matchCount)
			ci->returnValue->SetLong(m_groupcount[index]);
	else
		ci->returnValue->SetLong(-1);
	return pbxr;
}

PBXRESULT PbniRegex::Match(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;
	int matchLen, matchLenW;
	long index = ci->pArgs->GetAt(0)->GetLong() - 1; //in PB the index starts at 1

	if(index >= 0 && index <= m_matchCount)
	{
		//extract the match from the data
		matchLen = m_matchinfo[index][1] - m_matchinfo[index][0] + 1;
		LPSTR match = (LPSTR)malloc(matchLen + 1);
		lstrcpynA(match, (LPCSTR)(m_sData + m_matchinfo[index][0]), matchLen);
		//convert in WC
		matchLenW = mbstowcs(NULL, match, strlen(match)+1);
		LPWSTR wstr = (LPWSTR)malloc((matchLenW+1) * sizeof(wchar_t));
		mbstowcs(wstr, match, strlen(match)+1);

		// return value
		ci->returnValue->SetString(wstr);
		free(wstr);
		free(match);
	}
	else
		ci->returnValue->SetString(_T(""));
	return pbxr;
}

PBXRESULT PbniRegex::Group(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;
	int groupLen, groupLenW;

	if(ci->pArgs->GetCount() != 2)
		return PBX_E_INVOKE_WRONG_NUM_ARGS;

	long matchindex = ci->pArgs->GetAt(0)->GetLong() - 1; //in PB the index starts at 1

	if(matchindex >= 0 && matchindex <= m_matchCount)
	{
		long groupindex = ci->pArgs->GetAt(1)->GetLong() - 1;
		if(groupindex >= 0 && groupindex <= m_groupcount[matchindex]){
			if(-1 == m_matchinfo[matchindex][2*groupindex]){
				//the group matched nothing
				ci->returnValue->SetString(_T(""));
			}
			else {
				//extract the match from the data
				groupLen = m_matchinfo[matchindex][2*groupindex + 1] - m_matchinfo[matchindex][2*groupindex] + 1;
				LPSTR group = (LPSTR)malloc(groupLen + 1);
				lstrcpynA(group, (LPCSTR)(m_sData + m_matchinfo[matchindex][2*groupindex]), groupLen);
				//convert in WC
				groupLenW = mbstowcs(NULL, group, strlen(group)+1);
				LPWSTR wstr = (LPWSTR)malloc((groupLenW+1) * sizeof(wchar_t));
				mbstowcs(wstr, group, strlen(group)+1);

				// return value
				ci->returnValue->SetString(wstr);
				free(wstr);
				free(group);
			}
		}
		else
			ci->returnValue->SetString(_T(""));
	}
	else
		ci->returnValue->SetString(_T(""));
	return pbxr;
}

PBXRESULT PbniRegex::Replace(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	if(ci->pArgs->GetCount() != 2)
		return PBX_E_INVOKE_WRONG_NUM_ARGS;
	
	if(ci->pArgs->GetAt(0)->IsNull() || ci->pArgs->GetAt(1)->IsNull())
		ci->returnValue->SetToNull();
	else {
		//search string
		pbstring search = ci->pArgs->GetAt(0)->GetString();
		LPCTSTR searchStr = m_pSession->GetString(search);
		int searchLen = wcstombs(NULL, (LPWSTR)searchStr, 0) + 2;
		LPSTR search_utf8 = (LPSTR)malloc(searchLen);
		if (m_butf8)
		{
			WideCharToMultiByte(CP_UTF8,0,searchStr,-1,search_utf8,searchLen,NULL,NULL);	
		}
		else
			WideCharToMultiByte(CP_ACP,0,searchStr,-1,search_utf8,searchLen,NULL,NULL);	

		//replace string
		pbstring replace = ci->pArgs->GetAt(0)->GetString();
		LPCTSTR replaceStr = m_pSession->GetString(replace);
		int repLen = wcstombs(NULL, (LPWSTR)replaceStr, 0) + 2;
		LPSTR rep_utf8 = (LPSTR)malloc(repLen);
		if (m_butf8)
		{
			WideCharToMultiByte(CP_UTF8,0,replaceStr,-1,rep_utf8,repLen,NULL,NULL);	
		}
		else
			WideCharToMultiByte(CP_ACP,0,replaceStr,-1,rep_utf8,repLen,NULL,NULL);	

		//pcrecpp::RE(std::string(m_sPattern));

		free(search_utf8);
		free(rep_utf8);
	}
	return pbxr;
}