
// String manipulation functions for handling wide chars, utf and stuff alike
//
// @author : Sebastien Kirche - 2011

#include <string>

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
LPSTR WCToAnsi(LPCWSTR wcStr);

//UTF-8 <-> Wide char conversion
LPWSTR Utf8ToWC(LPCSTR AnsiStr);
LPSTR WCToUtf8(LPCWSTR wcStr);

int strlen_utf8(const unsigned char *ptr);
int strnlen_utf8(const unsigned char* ptr, unsigned int maxbytes);

extern char dbgMsg[2048];
