
// String manipulation functions for handling wide chars, utf and stuff alike
//
// @author : Sebastien Kirche - 2011

#ifdef PB9
typedef LPCSTR CSTR_PTR;
#define STR_CMP(s1,s2) strcmp(s1, s2)
#define STR_LEN(s,c) strlen(s)

#else
typedef LPCTSTR CSTR_PTR;
#define STR_CMP(s1,s2) wcscmp(s1, s2)
#define STR_LEN(s,c) WideCharToMultiByte(c,0,s,-1,NULL,0,NULL,NULL)

#endif

