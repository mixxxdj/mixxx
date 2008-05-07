/**********************************************
 *
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

#include "../globaldefs.h"
#include "http.h"

#ifdef __PROTOCOL_C__
/* BUILT-IN PROTOMSG TYPES */
const int PROTOMSGTYPE_LIBNAME = 0;
const int PROTOMSGTYPE_LIBVERSION = 1;
const int PROTOMSGTYPE_LIBOS = 2;
const int PROTOMSGTYPE_LIBARCH = 3;
const int PROTOMSGTYPE_LIBBUILDDATE = 4;
const int PROTOMSGTYPE_LIBCLIENT = 5;
const int PROTOMSGTYPE_CPUINFO = 0x080000000;
const int PROTOMSGTYPE_MEMINFO = 0x080000001;
const int PROTOMSGTYPE_FSINFO = 0x080000002;
const int PROTOMSGTYPE_OSINFO = 0x080000003;
const int PROTOMSGTYPE_RELEASE_ID = 0x080000004;
const int PROTOMSGTYPE_USER_ID = 0x080000005;

/* Crash Report PROTOMSG TYPES */
const char* PROTOMSG_CRASHREPORT_EXCEPTIONTYPE = "Exception Type\0";
const char* PROTOMSG_CRASHREPORT_CRASHADDR = "Exception Address\0";
const char* PROTOMSG_CRASHREPORT_STATE = "State\0";
const char* PROTOMSG_CRASHREPORT_BACKTRACE = "Back Trace\0";

/* PROTOCOL TOKENS */
const char TOK_DELIM = ':'; //NOTE: escaping code assumes this is a valid 7-bit ASCII char
const char TOK_BIN_DELIM = 'b'; //apears after the type string
const char TOK_ESCAPE = '\\';
const char *PROTOCOL_SYSINFO_HEADER = "SYSINFO\0";
const char *PROTOCOL_DBG_HEADER = "DEBUG\0";
const char *PROTOCOL_STD_HEADER = "STANDARD\0";
const char *PROTOCOL_CRASHREPORT_HEADER = "CRASH\0";
const char *PROTOCOL_CRASHDUMP_HEADER = "CRASHDUMP\0";

const char *PROTOCOL_SENDDUMP_BASIC = "DumpBasic\0";
const char *PROTOCOL_SENDDUMP_DATA = "DumpWithData\0";
const char *PROTOCOL_SENDDUMP_ALL = "DumpAll\0";

#endif //__PROTOCOL_C__

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

typedef struct _PROTOMSG{
    int type;
    union {
        char *pstz;
        BYTE *pb;
    };
    int size;   //if non-zero then its a binary msg
    struct _PROTOMSG *pnext;
} PROTOMSG;

typedef struct _DBGSTACKINFO{
	void *PC;
	const char *symbol;
} DBGSTACKINFO;

enum CrashActions{
	DUMPNOTHING,
	DUMPBASIC,
	DUMPWITHDATA,
	DUMPALL
};

class Protocol
{
public:
    Protocol(unsigned int msgMax, unsigned int msgDbgMax, const char *pstzReleaseID, const char *pstzUserID);
    ~Protocol();
    int flushBuff();    //send datapacket

	BOOL writeMsg(int type, const char *pstz, int msgScope = MSGSCOPE_STANDARD);	//ASCII C-String
	BOOL writeMsgUTF8(int type, const char *pstz, int msgScope = MSGSCOPE_STANDARD); //UTF8
	BOOL writeMsgUTF16(int type, const XCHAR *pstz, int msgScope = MSGSCOPE_STANDARD); //UTF16
	BOOL writeMsgUTF32(int type, const unsigned int *pstz) {return FALSE;};//UTF32
    BOOL writeMsgBin(int type, const BYTE *pb, unsigned int size, int msgScope = MSGSCOPE_STANDARD); //BINARY data
    
//    BOOL writeMsgDbgUTF8(const char *data);
    BOOL sendMsgDbg();

	static CrashActions writeCrashData(const char *pstzSessionID, long exceptionType, void *pcrashAddress, DBGSTACKINFO *pstackInfo, int cStackInfo, BYTE *pstate, int stateLen);
	static BOOL writeCrashDump(const char *pstzSessionID, void *pcrashData, int size);
	inline const char *getSessionID() { return (m_psessionID) ? m_psessionID : "N/A"; };
private:
    unsigned int m_packetCount;
    char *m_psessionID;
    unsigned int m_psessionIDLength;

    BOOL _queueMsg(PROTOMSG *pmsg, int msgScope);
    BOOL _queueMsgStandard(PROTOMSG *pmsg);
    BOOL _queueMsgDebug(PROTOMSG *pmsg);

	BOOL _writeMsg(int type, char *pstz, int msgScope);
    BOOL _sendSystemInfo(const char *pstzReleaseID, const char *pstzUserID);
    BOOL _escapeDelims(char **ppstz);
    int _flushBuff(Http *http, PROTOMSG **ppmsgTop);

    unsigned int m_cmsg;
    unsigned int m_cmsgMax;
    PROTOMSG *m_pmsgTop;
    PROTOMSG *m_pmsgLast;

    /* Debug messages, we use this as a stack and flush old messages after
     * m_cmsgDbgMax is reached.
     *
     * These are not TX'd automatically, you must explicitly TX them through
     * sendMsgDbg()
     */
    unsigned int m_cmsgDbg;
    unsigned int m_cmsgDbgMax;
    PROTOMSG *m_pmsgDbg;

    BOOL m_fFatalError;
};

#endif //__PROTOCOL_H__

