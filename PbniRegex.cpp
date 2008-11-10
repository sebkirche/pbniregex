// PbniRegex.cpp : PBNI class
//
// @author : Sebastien Kirche - 2008

#define _CRT_SECURE_NO_DEPRECATE
#define PCRE_STATIC 1

#include <stdio.h>
#include <string>
#include "PbniRegex.h"
#include "pcre.h"
#include "pcrecpp.h"

char dbgMsg[512];

PbniRegex::PbniRegex()
{
}

PbniRegex::PbniRegex( IPB_Session * pSession )
:m_pSession( pSession )
{

#ifdef _DEBUG
	OutputDebugString(_T("PbniRegex :: Constructor"));
#endif
	m_bGlobal = false;
	m_bCaseSensitive = false;
	m_butf8 = false;
	m_bmultiLine = false;
	m_sPattern = NULL;
	m_sData = NULL;
	re = NULL;
	m_matchCount = 0;
}

PbniRegex::~PbniRegex()
{
#ifdef _DEBUG
	OutputDebugString(_T("PbniRegex :: Destructor"));
#endif
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
		case mid_GroupPos:
			pbxr = this->GroupPos(ci);
			break;
		case mid_GroupLen:
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
#ifdef _DEBUG
	OutputDebugStringA(pcre_version());
#endif
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
		if(!m_bmultiLine)
			opts += PCRE_MULTILINE;

		re = pcre_compile(
				m_sPattern,           /* pattern */
				opts,                 /* options (défaut = 0)*/
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

/* Analyze the regexp and find out if extra info can be extracted to speed up repetitive calls to it 
   from PCRE documentation (7.3) :
   At present, studying a pattern is useful only for non-anchored patterns that do not have a single fixed starting character.
   A bitmap of possible starting bytes is created. 
*/
PBXRESULT PbniRegex::Study(PBCallInfo *ci)
{
	const char *error;
	PBXRESULT pbxr = PBX_OK;
	
	studinfo = pcre_study(re, 0, &error);
	if (error){
	  sprintf(dbgMsg, "PCRE study failed with message '%s'\n", error);
	  OutputDebugStringA(dbgMsg);
	}
	if (studinfo == NULL){
		//if the study did not found useful data
		ci->returnValue->SetBool(false);
	}else{
		//if the study was successful
		ci->returnValue->SetBool(true);
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
		  testStr_utf8,         /* the subject string */
		  testLen-2,			/* the length of the subject */
		  0,                    /* start at offset 0 in the subject */
		  0,                    /* default options */
		  ovector,              /* output vector for substring information */
		  OVECCOUNT);           /* number of elements in the output vector */
		if(rc < 0)
		{
			switch(rc)
			{
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
	}
	return pbxr;
}

PBXRESULT PbniRegex::SetUtf(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	m_butf8 = ci->pArgs->GetAt(0)->GetBool();
	return pbxr;
}

PBXRESULT PbniRegex::SetMulti(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	m_bmultiLine = ci->pArgs->GetAt(0)->GetBool();
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
		//until there is no match left OR if we did not set the global attribute for a single search
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

PBXRESULT PbniRegex::IsMulti(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	ci->returnValue->SetBool(m_bmultiLine);
	return pbxr;
}

PBXRESULT PbniRegex::IsUtf(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	ci->returnValue->SetBool(m_butf8);
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

PBXRESULT PbniRegex::GroupPos(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	if(ci->pArgs->GetCount() != 2)
		return PBX_E_INVOKE_WRONG_NUM_ARGS;

	long matchindex = ci->pArgs->GetAt(0)->GetLong() - 1; // for PB, first match is 1

	if(matchindex >= 0 && matchindex <= m_matchCount)
	{
		long groupindex = ci->pArgs->GetAt(1)->GetLong();//group 0 is the whole match
		if(groupindex >= 0 && groupindex <= m_groupcount[matchindex])
			ci->returnValue->SetLong(m_matchinfo[matchindex][2 * groupindex] + 1);
		else
			ci->returnValue->SetLong(-1);
	}
	else
		ci->returnValue->SetLong(-1);
	return pbxr;
}

PBXRESULT PbniRegex::GroupLen(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	if(ci->pArgs->GetCount() != 2)
		return PBX_E_INVOKE_WRONG_NUM_ARGS;

	long matchindex = ci->pArgs->GetAt(0)->GetLong() - 1; // for PB, first match is 1

	if(matchindex >= 0 && matchindex <= m_matchCount)
	{
		long groupindex = ci->pArgs->GetAt(1)->GetLong();//group 0 is the whole match
		if(groupindex >= 0 && groupindex <= m_groupcount[matchindex])
			ci->returnValue->SetLong(m_matchinfo[matchindex][2 * groupindex + 1] - m_matchinfo[matchindex][2 * groupindex]);
		else
			ci->returnValue->SetLong(-1);
	}
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
		ci->returnValue->SetToNull();
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
		long groupindex = ci->pArgs->GetAt(1)->GetLong(); //the group 0 is the whole match
		if(groupindex >= 0 && groupindex <= m_groupcount[matchindex]){
			if(-1 == m_matchinfo[matchindex][2*groupindex]){
				//the group matched nothing
				ci->returnValue->SetToNull();
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
			ci->returnValue->SetToNull();
	}
	else
		ci->returnValue->SetToNull();
	return pbxr;
}

PBXRESULT PbniRegex::Replace(PBCallInfo *ci)
{
	int nmatch = 0;
	int maxmatch = MAXMATCHES;
	int startoffset = 0;
	int res;
	int mvector[OVECCOUNT];
	char toexp[10];
	PBXRESULT pbxr = PBX_OK;

	if(ci->pArgs->GetCount() != 2)
		return PBX_E_INVOKE_WRONG_NUM_ARGS;
	
	if(ci->pArgs->GetAt(0)->IsNull() || ci->pArgs->GetAt(1)->IsNull())
		ci->returnValue->SetToNull();
	else {
		//search string utf-16 -> ansi / utf-8
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

		//replace string utf-16 -> ansi / utf-8
		pbstring replace = ci->pArgs->GetAt(1)->GetString();
		LPCTSTR replaceStr = m_pSession->GetString(replace);
		int repLen = wcstombs(NULL, (LPWSTR)replaceStr, 0) + 2;
		LPSTR rep_utf8 = (LPSTR)malloc(repLen);
		if (m_butf8)
		{
			WideCharToMultiByte(CP_UTF8,0,replaceStr,-1,rep_utf8,repLen,NULL,NULL);	
		}
		else
			WideCharToMultiByte(CP_ACP,0,replaceStr,-1,rep_utf8,repLen,NULL,NULL);	

		using namespace std;
		string working (search_utf8);
		do {
			//parcours les matches
			res = pcre_exec(re, NULL, working.c_str(), strlen(working.c_str()), startoffset, 0, mvector, OVECCOUNT);
			if (res > 0) {
				//m_groupcount[nmatch] = res - 1;
				basic_string<char> rep(rep_utf8);
				//expansion des substrings
				for(int j = 0; j < res; j++) // res = nb groups + 1
				{
					sprintf(toexp, "\\%d", j);
					int p;
					while((p = rep.find(toexp)) != string::npos)
						rep.replace(p,strlen(toexp), working.substr(mvector[(j)*2], mvector[(j)*2 +1]-mvector[(j)*2]));
				}
				//remplacement du match par la chaine de remplacement
				working.replace(mvector[0], mvector[1]-mvector[0], rep);
				startoffset = mvector[0] + rep.length();
				nmatch++;
			} else
				break;
		//until there is no match left OR if we did not set the global attribute for a single replacement
		}while (m_bGlobal && nmatch < maxmatch);

		//result string  utf-8 / ansi -> utf-16
		int outLen = mbstowcs(NULL, working.c_str(), strlen(working.c_str())+1);
		LPWSTR wstr = (LPWSTR)malloc((outLen+1) * sizeof(wchar_t));
		mbstowcs(wstr, working.c_str(), strlen(working.c_str())+1);

		ci->returnValue->SetString(wstr);

		free(search_utf8);
		free(rep_utf8);
		free(wstr);
	}
	return pbxr;
}

PBXRESULT PbniRegex::FastReplace(PBCallInfo *ci)
{
	PBXRESULT pbxr = PBX_OK;

	if(ci->pArgs->GetCount() != 3)
		//if parameter count  <> 3 -> error to PB
		return PBX_E_INVOKE_WRONG_NUM_ARGS;
	if(ci->pArgs->GetAt(0)->IsNull() || ci->pArgs->GetAt(1)->IsNull() || ci->pArgs->GetAt(2)->IsNull())
		//if at less 1 param is null, return null
		ci->returnValue->SetToNull();
	else
	{
		using namespace std; //for std::wstring
		bool casesensitive = FALSE;
		pbstring source = ci->pArgs->GetAt(0)->GetString();
		pbstring pattern = ci->pArgs->GetAt(1)->GetString();

		LPCTSTR s = m_pSession->GetString(source);
		LPCTSTR p = m_pSession->GetString(pattern);
		wstring sourcew(s);
		wstring patternw(p);
		//test for one occurence
		if(wcsstr(s, p)){
			
			pbstring replace = ci->pArgs->GetAt(2)->GetString();
			wstring replacew(m_pSession->GetString(replace));

			//here is the 'all' of 'replaceall' : replace each occurence
			int p = 0, startoffset = 0;
			while((p = sourcew.find(patternw, startoffset)) != string::npos){	
				sourcew.replace(p, patternw.length(), replacew);
				startoffset = p + replacew.length();
			}
			//return the resulting string
			ci->returnValue->SetString(sourcew.c_str());
		}
		else
			//if no occurrence, return the given string
			ci->returnValue->SetPBString(source);
	}
	return pbxr;
}

PBXRESULT PbniRegex::FastReplaceCase(PBCallInfo *ci)
{
	using namespace std; //for std::wstring
	pbstring source = ci->pArgs->GetAt(0)->GetString();
	pbstring pattern = ci->pArgs->GetAt(1)->GetString();
	pbstring replace = ci->pArgs->GetAt(2)->GetString();

	wstring sourcew(m_pSession->GetString(source));
	wstring patternw(m_pSession->GetString(pattern));
	wstring replacew(m_pSession->GetString(replace));

	int p = 0, startoffset = 0;
	//test for one occurence
	if((p = sourcew.find(patternw, startoffset)) != string::npos){
		//here is the 'all' of 'replaceall' : replace each occurence
		do{	
			sourcew.replace(p, patternw.length(), replacew);
			startoffset = p + replacew.length();
		}while((p = sourcew.find(patternw, startoffset)) != string::npos);
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
struct traits_nocase : std::char_traits<char>
{
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
inline std::ostream& operator<< ( std::ostream& stm, const string_nocase& str )
{
	return stm << reinterpret_cast<const std::string&>(str); 
}

inline std::istream& operator>> ( std::istream& stm, string_nocase& str )
{
	std::string s ; stm >> s ;
	if(stm) str.assign(s.begin(),s.end()) ;
	return stm ;
}

inline std::istream& getline( std::istream& stm, string_nocase& str )
{
	std::string s ; std::getline(stm,s) ;
	if(stm) str.assign(s.begin(),s.end()) ;
	return stm ;
}

// case insensitive character traits
// inherited copy (preserves case),
// case insensitive comparison, search
struct traitws_nocase : std::char_traits<wchar_t>
{
	static bool eq( const wchar_t& c1, const wchar_t& c2 ) { return toupper(c1) == toupper(c2) ; }
	static bool lt( const wchar_t& c1, const wchar_t& c2 ) { return toupper(c1) < toupper(c2) ; }
	static int compare( const wchar_t* s1, const wchar_t* s2, size_t N )
	{
	  //return strncasecmp( s1, s2, N ) ; // posix
	  return _wcsnicmp( s1, s2, N ) ; //microsoft
	}
	static const wchar_t* find( const wchar_t* s, size_t N, const wchar_t& a )
	{
	  for( size_t i=0 ; i<N ; ++i )
		if( toupper(s[i]) == toupper(a) ) return s+i ;
	  return 0 ;
	}
	static bool eq_int_type ( const int_type& c1, const int_type& c2 ) { return toupper(c1) == toupper(c2) ; }
};
// string preserves case; comparisons are case insensitive
typedef std::basic_string< wchar_t, traitws_nocase > wstring_nocase ;

// make wstring_nocase work like a std::wstring
//           with streams using std::char_traits
// std::basic_istream< char, std::char_traits<char> > (std::istream) and
// std::basic_ostream< char, std::char_traits<char> > (std::ostream)
inline std::wostream& operator<< ( std::wostream& stm, const wstring_nocase& str )
{
	return stm << reinterpret_cast<const std::wstring&>(str); 
}

inline std::wistream& operator>> ( std::wistream& stm, wstring_nocase& str )
{
	std::wstring s ; stm >> s ;
	if(stm) str.assign(s.begin(),s.end()) ;
	return stm ;
}

inline std::wistream& getline( std::wistream& stm, wstring_nocase& str )
{
	std::wstring s ; std::getline(stm,s) ;
	if(stm) str.assign(s.begin(),s.end()) ;
	return stm ;
}


PBXRESULT PbniRegex::FastReplaceNoCase(PBCallInfo *ci)
{
	using namespace std; //for std::wstring
	pbstring source = ci->pArgs->GetAt(0)->GetString();
	pbstring pattern = ci->pArgs->GetAt(1)->GetString();
	pbstring replace = ci->pArgs->GetAt(2)->GetString();

	wstring_nocase sourcew(m_pSession->GetString(source));
	wstring_nocase patternw(m_pSession->GetString(pattern));
	wstring_nocase replacew(m_pSession->GetString(replace));

	int p = 0, startoffset = 0;
	//test for one occurence
	if((p = sourcew.find(patternw, startoffset)) != string_nocase::npos){
		//here is the 'all' of 'replaceall' : replace each occurence
		do{	
			sourcew.replace(p, patternw.length(), replacew);
			startoffset = p + replacew.length();
		}while((p = sourcew.find(patternw, startoffset)) != string_nocase::npos);
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
	else
	{
		using namespace std; //for std::wstring
		bool casesensitive = FALSE;

		casesensitive = ci->pArgs->GetAt(3)->GetBool();
		if(casesensitive)
			pbxr = FastReplaceCase(ci);
		else
			pbxr = FastReplaceNoCase(ci);
	}
	return pbxr;
}

