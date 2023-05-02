/**
  @file
  @author Stefan Frings
*/

#ifndef LOGGLOBAL_H
#define LOGGLOBAL_H

#include <QtGlobal>

// This is specific to Windows dll's
#if defined(Q_OS_WIN)
    #if defined(QTWEBAPPLIB_EXPORT)
        #define DECLSPEC Q_DECL_EXPORT
    #elif defined(QTWEBAPPLIB_IMPORT)
        #define DECLSPEC Q_DECL_IMPORT
    #endif
#endif
#if !defined(DECLSPEC)
    #define DECLSPEC
#endif

#if __cplusplus < 201103L
    #define nullptr 0
#endif

#endif // LOGGLOBAL_H

