#ifndef __BSTRING_H__
#define __BSTRING_H__

#ifdef UNICODE
typedef wchar_t bchar;
#define bstrcpy wcscpy
#define bstrcat wcscat
#define bstrcmp wcscmp
#define bsprintf wsprintf
#define bstrcpyn wcsncpy
#define bstrtol wcstol
#define bstrdup wcsdup
#define bstrrchr wcsrchr

extern void qudConvert(wchar_t * dst, char const * src, unsigned int len);
extern void qudConvert(char * dst, wchar_t const * src, unsigned int len);
#define BCONVERT(t,s,dest,src) { t tmp_##dest [s]; qudConvert(tmp_##dest, src, s); dest = tmp_##dest; }

#else
typedef char bchar;
#define bstrcpy strcpy
#define bstrcmp strcmp
#define bstrcat strcat
#define bsprintf sprintf
#define bstrcpyn lstrcpyn
#define bstrtol strtol
#define bstrdup strdup
#define bstrrchr strrchr

#endif

#endif
