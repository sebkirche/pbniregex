
// String manipulation functions for handling wide chars, utf and stuff alike
//
// @author : Sebastien Kirche - 2011, 2013

#include "pbniregex.h"

// Convert an ANSI encoded string into a wide char string
// /!\ Do not forget to free the allocated string !!!
LPCWSTR MultiBStrToWC(LPCSTR AnsiStr, UINT encoding){
	int strLen = MultiByteToWideChar(encoding, 0, AnsiStr, -1, 0, 0);
	LPWSTR wstr = (LPWSTR)malloc((strLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(encoding, 0, AnsiStr, -1, wstr, strLen);
	return wstr;
}

// Convert a wide char string into an ANSI encoded string
// /!\ Do not forget to free the allocated string !!!
LPCSTR WCToMultiBStr(LPCWSTR wcStr, UINT encoding){
	int strLen = WideCharToMultiByte(encoding, 0, wcStr, -1, 0, 0, 0, 0);
	LPSTR str = (LPSTR)malloc(strLen + 1);
	WideCharToMultiByte(encoding, 0, wcStr, -1, str, strLen, 0, 0);
	return str;
}

//ANSI -> Wide char
LPCWSTR AnsiToWC(LPCSTR AnsiStr){
	return MultiBStrToWC(AnsiStr, CP_ACP);
}

//Wide char -> ANSI
LPCSTR WCToAnsi(LPCWSTR wcStr){
	return WCToMultiBStr(wcStr, CP_ACP);
}

//UTF-8 -> Wide char
LPCWSTR Utf8ToWC(LPCSTR AnsiStr){
	return MultiBStrToWC(AnsiStr, CP_ACP);
}

//Wide char -> UTF-8
LPCSTR WCToUtf8(LPCWSTR wcStr){
	return WCToMultiBStr(wcStr, CP_ACP);
}
