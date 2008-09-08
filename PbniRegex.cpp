// PbniRegex.cpp : PBNI class
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include "PbniRegex.h"
#include "pcre.h"

char dbgMsg[512];
#define OVECCOUNT 30    /* should be a multiple of 3 */

// default constructor
PbniRegex::PbniRegex()
{
}

PbniRegex::PbniRegex( IPB_Session * pSession )
:m_pSession( pSession )
{
	OutputDebugString(_T("PbniRegex :: Constructor"));
	m_bGlobal = false;
	m_bCaseSensitive = false;
	m_butf8 = false;
	m_sPattern = NULL;
	re = NULL;
}

// destructor
PbniRegex::~PbniRegex()
{
	OutputDebugString(_T("PbniRegex :: Destructor"));
	if(m_sPattern)
		free(m_sPattern);
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
		default:
			pbxr = PBX_E_INVOKE_METHOD_AMBIGUOUS;
	}
	return pbxr;
}


void PbniRegex::Destroy() 
{
   delete this;
}

// Method callable from PowerBuilder
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

	if(ci->pArgs->GetAt(0)->IsNull())
	{
		// if any of the passed arguments is null, return the null value
		ci->returnValue->SetBool(false);
	}
	else
	{
		pbstring test = ci->pArgs->GetAt(0)->GetString();
		LPCTSTR pattern_ucs2 = m_pSession->GetString(test);
		//char *pattern_utf8;
		int patternLen = wcstombs(NULL, (LPWSTR)pattern_ucs2, 0) + 2;
		m_sPattern = (LPSTR)malloc(patternLen);
		if (m_butf8)
		{
			WideCharToMultiByte(CP_UTF8,0,pattern_ucs2,-1,m_sPattern,patternLen,NULL,NULL);	
			opts += PCRE_UTF8;
		}
		else
			WideCharToMultiByte(CP_ACP,0,pattern_ucs2,-1,m_sPattern,patternLen,NULL,NULL);	

		if(!m_bCaseSensitive)
			opts += PCRE_CASELESS;

		re = pcre_compile(
				m_sPattern,              /* the pattern */
				opts,                    /* default options */
				&error,               /* for error message */
				&erroffset,           /* for error offset */
				NULL);                /* use default character tables */
		if (re == NULL)
		{
		  sprintf(dbgMsg, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
		  ci->returnValue->SetBool(false);
		}
		else
		{
			ci->returnValue->SetBool(true);
		}
	}
	return pbxr;
}

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
	PBXRESULT pbxr = PBX_OK;

	return pbxr;
}
