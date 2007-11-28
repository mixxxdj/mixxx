/**********************************************
 * Cmetrics.h - Case Metrics Interface
 *  Copyright 2007 John Sully.
 *
 *  This file is part of Case Metrics.
 *
 *  Case Metrics is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  Case Metrics is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Case Metrics.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************/

/***********************************************
 *  Notice of Use:
 *     While the Case Metrics library is free software;
 *     You must obtain permission to use the official
 *     case metrics servers.  See casemetrics.com for details
 *     on obtaining such permission.  Alternatively you may
 *     use the library to contact your own servers.
 ***********************************************/

#ifndef __CMETRICS_H__
#define __CMETRICS_H__

/* Compiler specific export prefix */
#ifndef CC_EXPORT_PREFIX
#ifdef _MSC_VER
#define CC_EXPORT_PREFIX _declspec (dllexport)
#elif __GNUC__
#define CC_EXPORT_PREFIX 
#else //__GNUC__
#error "Unsupported Compiler: You must manually define CC_EXPORT_PREFIX"
#endif
#endif

/* User Specified Settings */
#define USER_DEBUG_C qDebug /* User Defined Debug Code Here */
#define USER_DEBUG_CPP qDebug /* User Defined Debug Code Here */

/* Library Data-Types */
typedef char CM_UTF8;
typedef unsigned short CM_UTF16;

#ifdef __cplusplus 
extern "C"{
#endif
	/* Initialization Routines */
	CC_EXPORT_PREFIX int cm_init(int maxMsgQueue, int maxDbgMsg, int fuserVerified);
	CC_EXPORT_PREFIX void cm_close(int timeout);

	/* Low Level Message Routines */
	CC_EXPORT_PREFIX void cm_writemsg_ascii(int msgType, char *pstz);
	CC_EXPORT_PREFIX void cm_writemsg_utf8(int msgType, CM_UTF8 *pstz);
	CC_EXPORT_PREFIX void cm_writemsg_utf16(int msgType, CM_UTF16 pstz);
	CC_EXPORT_PREFIX void cm_writemsg_bin(int msgType, char *pb, unsigned int size);

	/* Debug Routines */
	CC_EXPORT_PREFIX void cm_writemsgDbg(const char *, ...);
	CC_EXPORT_PREFIX void cm_writemsgDbg_utf8(const CM_UTF8 *, ...);
	CC_EXPORT_PREFIX void cm_writemsgDbg_utf16(const CM_UTF16 *, ...);

	CC_EXPORT_PREFIX void cm_writemsgWarn(const char *, ...);

	CC_EXPORT_PREFIX void cm_writemsgFail(const char *, ...);
#ifdef __cplusplus 
}
#endif

#undef CC_EXPORT_PREFIX

#endif //__CMETRICS_H__
