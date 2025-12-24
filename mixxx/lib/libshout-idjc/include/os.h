#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define strdup _strdup

typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8  uint8_t;
typedef __int32 int32_t;
typedef int  ssize_t;
typedef int socklen_t;
#ifndef _WIN64
// NOTE(rryan): For some reason, x64 already had size_t defined but x86 doesn't. 
typedef unsigned int size_t;
#endif
#endif
