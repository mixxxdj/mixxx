/*************************** asmlib.h ***************************************
* Author:        Agner Fog
* Date created:  2003
* Last modified: 2009-07-20
* Project:       asmlib.zip
* Source URL:    www.agner.org/optimize
*
* Description:
* Header file for the asmlib function library.
* This library is available in many versions for different platforms.
* See asmlib-instructions.pdf for details.
*
* Copyright 2003 - 2009 by Agner Fog. 
* GNU General Public License http://www.gnu.org/licenses/gpl.html
*****************************************************************************/


#ifndef ASMLIB_H
#define ASMLIB_H


/***********************************************************************
Define compiler-specific types and directives
***********************************************************************/

// Define type size_t
#ifndef _SIZE_T_DEFINED
#include "stddef.h"
#endif

// Define integer types with known size: int32_t, uint32_t, int64_t, uint64_t.
// If this doesn't work then insert compiler-specific definitions here:
#if defined(__GNUC__)
  // Compilers supporting C99 or C++0x have inttypes.h defining these integer types
  #include <inttypes.h>
  #define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#elif defined(_WIN16) || defined(__MSDOS__) || defined(_MSDOS) 
   // 16 bit systems use long int for 32 bit integer
  typedef   signed long int int32_t;
  typedef unsigned long int uint32_t;
#elif defined(_MSC_VER)
  // Microsoft have their own definition
  typedef   signed __int32  int32_t;
  typedef unsigned __int32 uint32_t;
  typedef   signed __int64  int64_t;
  typedef unsigned __int64 uint64_t;
  #define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#else
  // This works with most compilers
  typedef signed int          int32_t;
  typedef unsigned int       uint32_t;
  typedef long long           int64_t;
  typedef unsigned long long uint64_t;
  #define INT64_SUPPORTED // Remove this if the compiler doesn't support 64-bit integers
#endif


// Turn off name mangling
#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
Function prototypes
***********************************************************************/

void * A_memcpy (void * dest, const void * src, size_t count); // Copy count bytes from src to dest
void * A_memmove(void * dest, const void * src, size_t count); // Same as memcpy, allows overlap between src and dest
void * A_memset (void * dest, int c, size_t count);            // Set count bytes in dest to (char)c
char * A_strcat (char * dest, const char * src);               // Concatenate strings dest and src. Store result in dest
char * A_strcpy (char * dest, const char * src);               // Copy string src to dest
size_t A_strlen (const char * str);                            // Get length of zero-terminated string
int    stricmp_az (const char *string1, const char *string2);  // Compare strings. Case insensitive for A-Z

extern size_t   CacheBypassLimit;                              // The above functions can bypass cache when writing memory bigger than this limit

int    InstructionSet(void);                                   // Tell which instruction set is supported
extern int IInstrSet;                                          // Set by first call to InstructionSet()
char * ProcessorName(void);                                    // ASCIIZ text describing microprocessor
void   Serialize();                                            // Serialize before and after __readpmc()
int    RoundD (double x);                                      // Round to nearest or even
int    RoundF (float  x);                                      // Round to nearest or even
#ifdef INT64_SUPPORTED
   uint64_t ReadTSC(void);                                     // Read microprocessor internal clock (64 bits)
#else
   uint32_t ReadTSC(void);                                     // Read microprocessor internal clock (only 32 bits supported by compiler)
#endif
void cpuid_ex (int abcd[4], int eax, int ecx);                 // call CPUID instruction
static inline void cpuid_abcd (int abcd[4], int eax) {
   cpuid_ex(abcd, eax, 0);}


#ifdef __cplusplus
}  // end of extern "C"

static inline int Round (double x) {   // Overload name Round
   return RoundD(x);}
static inline int Round (float  x) {   // Overload name Round
   return RoundF(x);}

#endif // __cplusplus

#endif // ASMLIB_H
