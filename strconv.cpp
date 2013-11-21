
// String manipulation functions for handling wide chars, utf and stuff alike
//
// @author : Sebastien Kirche - 2011, 2013

#include "pbniregex.h"

// Convert an ANSI encoded string into a wide char string
// /!\ Do not forget to free the allocated string !!!
LPWSTR MultiBStrToWC(LPCSTR AnsiStr, UINT encoding, int *wcLen){
	*wcLen = MultiByteToWideChar(encoding, 0, AnsiStr, -1, 0, 0);
	LPWSTR wstr = (LPWSTR)malloc((*wcLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(encoding, 0, AnsiStr, -1, wstr, *wcLen);
	return wstr;
}

// Convert a wide char string into an ANSI encoded string
// /!\ Do not forget to free the allocated string !!!
LPSTR WCToMultiBStr(LPCWSTR wcStr, UINT encoding, int *byteLen){
	*byteLen = WideCharToMultiByte(encoding, 0, wcStr, -1, 0, 0, 0, 0);
	LPSTR str = (LPSTR)malloc(*byteLen + 1);
	WideCharToMultiByte(encoding, 0, wcStr, -1, str, *byteLen, 0, 0);
	return str;
}

//ANSI -> Wide char
LPWSTR AnsiToWC(LPCSTR AnsiStr, int *wcLen){
	return MultiBStrToWC(AnsiStr, CP_ACP, wcLen);
}
LPWSTR AnsiToWC(LPCSTR AnsiStr){
	int wcLen;
	return MultiBStrToWC(AnsiStr, CP_ACP, &wcLen);
}

//Wide char -> ANSI
LPSTR WCToAnsi(LPCWSTR wcStr, int *byteLen){
	return WCToMultiBStr(wcStr, CP_ACP, byteLen);
}
LPSTR WCToAnsi(LPCWSTR wcStr){
	int byteLen;
	return WCToMultiBStr(wcStr, CP_ACP, &byteLen);
}

//UTF-8 -> Wide char
LPWSTR Utf8ToWC(LPCSTR AnsiStr, int *wcLen){
	return MultiBStrToWC(AnsiStr, CP_UTF8, wcLen);
}
LPWSTR Utf8ToWC(LPCSTR AnsiStr){
	int wcLen;
	return MultiBStrToWC(AnsiStr, CP_UTF8, &wcLen);
}

//Wide char -> UTF-8
LPSTR WCToUtf8(LPCWSTR wcStr, int *byteLen){
	return WCToMultiBStr(wcStr, CP_UTF8, byteLen);
}
LPSTR WCToUtf8(LPCWSTR wcStr){
	int byteLen;
	return WCToMultiBStr(wcStr, CP_UTF8, &byteLen);
}

//preliminary versions for an utf-8 compliant strlen()
int strlen_utf8(const unsigned char *ptr){
	int c = 0;
	while(*ptr){
		if((*ptr & 0xC0)!=0x80){
			c++;
		}
		ptr++;		
	}
	return c;
}

//utf-8 compliant strlen that can parse a limited amount of bytes
int strnlen_utf8(const unsigned char* ptr, unsigned int maxbytes){
	int c = 0;	
	while(*ptr && maxbytes){
		if((*ptr & 0xC0)!=0x80){
#ifdef _DEBUG
			_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "strnlen_utf8 c++ : %c ; %d ; %d ; MASKED = %02X\n", *ptr, c, maxbytes, (*ptr & 0xC0));
			OutputDebugStringA(dbgMsg);
#endif
			c++;
		}
		ptr++;		
		maxbytes--;
#ifdef _DEBUG
			_snprintf(dbgMsg, sizeof(dbgMsg) - 1, "strnlen_utf8 ptr++ : %c ; %d ; %d\n", *ptr, c, maxbytes);
			OutputDebugStringA(dbgMsg);
#endif
	}
	return c;
}

// Smart releaseString compatible PB10,10.5, and 11+
void CommonReleaseString(IPB_Session *sess, LPCWSTR str){
#if defined (PBVER) && (PBVER > 90) && (PBVER < 110)
	free((void*)str);
#elif defined (PBVER) && (PBVER > 105)
	sess->ReleaseString((LPWSTR)str);
#endif
}