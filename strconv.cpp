
// String manipulation functions for handling wide chars, utf and stuff alike
//
// @author : Sebastien Kirche - 2011

#include "pbniregex.h"

LPCWSTR AnsiStrToWC(LPCSTR AnsiStr){
	int strLen = MultiByteToWideChar(CP_ACP, 0, AnsiStr, -1, 0, 0);
	LPWSTR wstr = (LPWSTR)malloc((strLen + 1) * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, AnsiStr, -1, wstr, strLen);
	return wstr;
}

LPCSTR WCToAnsiStr(LPCWSTR wcStr){
	int strLen = WideCharToMultiByte(CP_ACP, 0, wcStr, -1, 0, 0, 0, 0);
	LPSTR str = (LPSTR)malloc(strLen + 1);
	WideCharToMultiByte(CP_ACP, 0, wcStr, -1, str, strLen, 0, 0);
	return str;
}
