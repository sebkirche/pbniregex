
// String manipulation functions for handling wide chars, utf and stuff alike
//
// @author : Sebastien Kirche - 2011

#include <string>
#include <PBEXT.H>

#if defined (PBVER) && (PBVER < 100)

#define STR(x) x

//typedef string stdstring;
//typedef basic_string<char, char_traits<char>, allocator<char> >	string;
typedef std::basic_string<char, std::char_traits<char>, std::allocator<char> >	stdstring;

#else

#define STR(x) _T(x)

//typedef wstring stdstring;
//typedef basic_string<wchar_t, char_traits<wchar_t>,	allocator<wchar_t> > wstring;
typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > stdstring;

#endif

//ANSI <-> Wide char conversion
LPWSTR AnsiToWC(LPCSTR AnsiStr);
LPWSTR AnsiToWC(LPCSTR AnsiStr, int *wcLen);
LPSTR WCToAnsi(LPCWSTR wcStr);
LPSTR WCToAnsi(LPCWSTR wcStr, int *byteLen);

//UTF-8 <-> Wide char conversion
LPWSTR Utf8ToWC(LPCSTR AnsiStr);
LPWSTR Utf8ToWC(LPCSTR AnsiStr, int *wcLen);
LPSTR WCToUtf8(LPCWSTR wcStr);
LPSTR WCToUtf8(LPCWSTR wcStr, int *byteLen);

int strlen_utf8(const unsigned char *ptr);
int strnlen_utf8(const unsigned char* ptr, unsigned int maxbytes);
void CommonReleaseString(IPB_Session *sess, LPCWSTR str);

extern char dbgMsg[2048];
