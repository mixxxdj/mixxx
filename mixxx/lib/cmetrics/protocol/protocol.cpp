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

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#define snprintf _snprintf_s
#endif

#define __PROTOCOL_C__
#include "protocol.h"
#undef __PROTOCOL_C__
#include "http.h"
#include "../utf/ConvertUTF.h"

extern "C"{
#include "../cpuinfo/cpuinfo.h"
#include "../fsinfo/fsinfo.h"
#include "../meminfo/meminfo.h"
#include "../osinfo/osinfo.h"
}

/* Returns TRUE if one string is a substring of the other */
inline int fsubStr(const char *pstz1, const char *pstz2)
{
    int i=0;
    while(pstz1[i] != '\0' && pstz2[i] != '\0')
    {
	if(pstz1[i] != pstz2[i])
	    return FALSE;
	i++;
    }
    return TRUE;
}



Protocol::Protocol(unsigned int msgMax, unsigned int msgDbgMax, const char *pstzReleaseID, const char *pstzUserID)
{
    m_fFatalError = false;

    m_cmsg = 0;
    m_cmsgMax = msgMax;
    m_pmsgTop = NULL;
    m_pmsgLast = NULL;

    m_psessionID = NULL;
    m_psessionIDLength = 0;
    m_packetCount = 1;

    m_cmsgDbg = 0;
    m_cmsgDbgMax = msgDbgMax;
    m_pmsgDbg = NULL;
    
    if( _sendSystemInfo(pstzReleaseID, pstzUserID) != OK)
        m_fFatalError = true;
}

Protocol::~Protocol()
{
    flushBuff();

    free(m_psessionID);
}

BOOL Protocol::_sendSystemInfo(const char *pstzReleaseID, const char *pstzUserID)
{    
    Http http(SERVER_HOST);
    XCHAR *pstz;
    int retVal;

    http.appendPostData(PROTOCOL_SYSINFO_HEADER, strlen(PROTOCOL_SYSINFO_HEADER));
    //Send Lib Data
    writeMsg(PROTOMSGTYPE_LIBNAME, LIB_NAME);
    writeMsg(PROTOMSGTYPE_LIBVERSION, LIB_VERSION);
    writeMsg(PROTOMSGTYPE_LIBOS, LIB_OS);
    writeMsg(PROTOMSGTYPE_LIBARCH, LIB_ARCH);
    writeMsg(PROTOMSGTYPE_LIBBUILDDATE, __DATE__);
    writeMsg(PROTOMSGTYPE_LIBCLIENT, LIB_CLIENT);

    //Send Release/User IDs
    writeMsg(PROTOMSGTYPE_RELEASE_ID, pstzReleaseID);
    if(strlen(pstzUserID) > 0)
        writeMsg(PROTOMSGTYPE_USER_ID, pstzUserID);

    //Send Module Data
    pstz = cpuInfoStz();
    writeMsgUTF16(PROTOMSGTYPE_CPUINFO, pstz);
    free(pstz);

    pstz = memInfoStz();
    writeMsgUTF16(PROTOMSGTYPE_MEMINFO, pstz);
    free(pstz);

    pstz = fsInfoStz();
    writeMsgUTF16(PROTOMSGTYPE_FSINFO, pstz);
    free(pstz);

	pstz = osInfoStz();
	writeMsgUTF16(PROTOMSGTYPE_OSINFO, pstz);
	free(pstz);

    _flushBuff(&http, &m_pmsgTop);
    retVal =  http.send();
    
    if(retVal == OK && http.m_rxpacketLength == 0)
	retVal = ERR_TEMP;

    if(retVal == OK)
    {
	//get sessionID size
	char *pchCur = http.m_rxpacket;
	int sessionIDLength = 0;
	int idx=0;
	while(*pchCur != TOK_DELIM && *pchCur != '\0')
	{
	    sessionIDLength++;
	    pchCur = pchCur + 1;
	}
	m_psessionID = (char*) malloc(sessionIDLength+1);
	while(idx < sessionIDLength)
	{
	    m_psessionID[idx] = http.m_rxpacket[idx];
	    idx++;
	}
	m_psessionID[sessionIDLength] = '\0';
	m_psessionIDLength = sessionIDLength;
    }
    else if(retVal == ERR_FATAL)
	m_fFatalError = true;

    return retVal;
}

BOOL  Protocol::writeCrashDump(const char *pstzSessionID, void *pcrashData, int size)
{
	Http http(SERVER_HOST);

	
	//Write Header Stuff
	http.appendPostData(PROTOCOL_CRASHDUMP_HEADER, strlen(PROTOCOL_CRASHDUMP_HEADER));
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	http.appendPostData((BYTE*) pstzSessionID, strlen(pstzSessionID));
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));

	http.appendPostData((BYTE*) pcrashData, size);

	return http.send();
}

/* Crash Data Reporting */
/* We Implement this as static so it can be called without a Protocol object */
/* We want execution of this to remain on the stack as we can't trust the heap */
CrashActions Protocol::writeCrashData(const char *pstzSessionID, long exceptionType, void *pcrashAddress, DBGSTACKINFO *pStackInfo, int cStackInfo, BYTE *pstate, int stateLen)
{
#define MAX_BUFSIZE 1024
	char buf[MAX_BUFSIZE];
	int bufOffset = 0;
	/* Send Crash Data */
	Http http(SERVER_HOST);

	//Write Header Stuff
	http.appendPostData(PROTOCOL_CRASHREPORT_HEADER, strlen(PROTOCOL_CRASHREPORT_HEADER));
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	http.appendPostData((BYTE*) pstzSessionID, strlen(pstzSessionID));

	//Write Lib Data
#define appendMsg(type, pstz) http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM)); \
							snprintf(buf, MAX_BUFSIZE, "%X", type); \
							http.appendPostData((BYTE*) buf, strlen(buf)); \
							http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM)); \
							http.appendPostData((BYTE*) pstz, strlen(pstz));
	appendMsg(PROTOMSGTYPE_LIBNAME, LIB_NAME);
    appendMsg(PROTOMSGTYPE_LIBVERSION, LIB_VERSION);
    appendMsg(PROTOMSGTYPE_LIBOS, LIB_OS);
    appendMsg(PROTOMSGTYPE_LIBARCH, LIB_ARCH);
    appendMsg(PROTOMSGTYPE_LIBBUILDDATE, __DATE__);
    appendMsg(PROTOMSGTYPE_LIBCLIENT, LIB_CLIENT);
#undef appendMsg

	//Write Exception Type
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	http.appendPostData((BYTE*) PROTOMSG_CRASHREPORT_EXCEPTIONTYPE, strlen(PROTOMSG_CRASHREPORT_EXCEPTIONTYPE));
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	snprintf(buf, MAX_BUFSIZE, "0x0%lX", exceptionType);
	http.appendPostData((BYTE*) buf, strlen(buf));

	//Write Crash Address
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	http.appendPostData((BYTE*) PROTOMSG_CRASHREPORT_CRASHADDR, strlen(PROTOMSG_CRASHREPORT_CRASHADDR));
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	snprintf(buf, MAX_BUFSIZE, "0x0%llX", (long long) pcrashAddress);
	http.appendPostData((BYTE*) buf, strlen(buf));

	//Write Crash State
#if 0
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	http.appendPostData((BYTE*) PROTOMSG_CRASHREPORT_STATE, strlen(PROTOMSG_CRASHREPORT_STATE));
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	http.appendPostData((BYTE*) pstate, stateLen);
#endif

	//Write Back Trace
	bufOffset = 0;
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	http.appendPostData((BYTE*) PROTOMSG_CRASHREPORT_BACKTRACE, strlen(PROTOMSG_CRASHREPORT_BACKTRACE));
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	for(int i=0; i < cStackInfo; i++)
	{
		snprintf(buf, MAX_BUFSIZE, "0x0%llX [%s]\n", (long long) pStackInfo[i].PC, pStackInfo[i].symbol);
		buf[MAX_BUFSIZE - 1] = '\0';
		http.appendPostData((BYTE*) buf, strlen(buf));
	}

	//Determine Results
	int retVal = http.send();
	if(retVal != OK)
	{
		return DUMPNOTHING;
	}
	int offset  = 0;	//this is a hack to handle extra line feed on beginning
	if(http.m_rxpacket[0] == 0x0A)
		offset = 1;
	if(fsubStr(http.m_rxpacket + offset, PROTOCOL_SENDDUMP_BASIC))
		return DUMPBASIC;
	if(fsubStr(http.m_rxpacket + offset, PROTOCOL_SENDDUMP_DATA))
		return DUMPWITHDATA;
	if(fsubStr(http.m_rxpacket + offset, PROTOCOL_SENDDUMP_ALL))
		return DUMPALL;

	return DUMPNOTHING;
#undef MAX_BUFSIZE
}
BOOL Protocol::sendMsgDbg()
{
    if(m_pmsgDbg != NULL)
    {
	Http http(SERVER_HOST);
	http.appendPostData((BYTE*) m_psessionID, m_psessionIDLength);
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	http.appendPostData(PROTOCOL_DBG_HEADER, strlen(PROTOCOL_DBG_HEADER));

	_flushBuff(&http, &m_pmsgDbg);
	if(http.send() == ERR_FATAL)
	    m_fFatalError = true;
    }

    return TRUE;
}

BOOL Protocol::_escapeDelims(char **ppstz)
{
    char *pstz = *ppstz;
    int idx = 0;
    int idxT = 0;
    int cdelim = 0;
    char *pstzT;
    //COUNT DELIMS
    idx = 0;
    while(pstz[idx] != '\0')
    {
	if(pstz[idx] == TOK_DELIM || pstz[idx] == TOK_ESCAPE)
	    cdelim++;
	idx++;
    }
    //ESCAPE CHARS
    if(cdelim > 0)
    {
	pstzT = (char*) malloc(strlen(pstz) + cdelim + 1);	//each delim will need an extra byte
	if(pstzT == NULL)
	    return 0;
	idx = 0;
	while(pstz[idx] != '\0')
	{
	    if(pstz[idx] == TOK_DELIM || pstz[idx] == TOK_ESCAPE)
	    {
		pstzT[idxT++] = TOK_ESCAPE;
	    }
	    pstzT[idxT++] = pstz[idx];
	    idx++;
	}
	pstzT[idxT] = '\0';
	*ppstz = pstzT; //swap the strings
	free(pstz);
    }
    return 1;
}

BOOL Protocol::writeMsgBin(int type, const BYTE *pb, unsigned int size, int msgScope)
{
    if(m_fFatalError)
	return FALSE;
    BYTE *buf = (BYTE*) malloc(size);
    PROTOMSG *pnewMsg = (PROTOMSG*) malloc(sizeof(PROTOMSG));

    memcpy(buf, pb, size);
    pnewMsg->type = type;
    pnewMsg->pb = buf;
    pnewMsg->size = size;
    pnewMsg->pnext = NULL;

    return _queueMsg(pnewMsg, msgScope);
}

/* Adds the UTF-8 buffer to the queue, this is a shallow copy */
BOOL Protocol::_writeMsg(int type, char *pstz, int msgScope)
{
    if(!m_fFatalError)
    {
	PROTOMSG *pnewMsg;

	_escapeDelims(&pstz);
#ifdef DEBUG
	UTF8 *pchEnd = (UTF8*) pstz;
	while(*pchEnd != '\0')
	    pchEnd++;

	assert(isLegalUTF8Sequence((UTF8*) pstz, pchEnd));
#endif

	pnewMsg = (PROTOMSG*) malloc(sizeof(PROTOMSG));
	pnewMsg->type = type;
	pnewMsg->pstz = pstz;
	pnewMsg->size = 0;	//is a string msg
	pnewMsg->pnext = NULL;

	return _queueMsg(pnewMsg, msgScope);
    }
    else
    {
	free(pstz);
	return FALSE;
    }
}

/* UTF8 Debug Message Write */
BOOL Protocol::_queueMsgDebug(PROTOMSG *pnewDbgMsg)
{
    /* Link to start of Dbg Msg List */
    pnewDbgMsg->pnext = m_pmsgDbg;
    m_pmsgDbg = pnewDbgMsg;
    m_cmsgDbg++;
    
    /* Free old MSGs if necessary */
    if(m_cmsgDbg > m_cmsgDbgMax)
    {
	unsigned int i;
	PROTOMSG *pmsgCur = m_pmsgDbg;
	PROTOMSG *pmsgTmp;
	for(i=0; i < (m_cmsgDbgMax - 1); i++)
	{
	    assert(pmsgCur != NULL);
	    pmsgCur = pmsgCur->pnext;
	}
	//pmsgCur and after need to be freed
	pmsgTmp = pmsgCur;
	pmsgCur = pmsgCur->pnext;
	pmsgTmp->pnext = NULL;
	assert(pmsgCur != m_pmsgDbg);	
	
	m_cmsgDbg = m_cmsgDbgMax;

	while(pmsgCur != NULL)
	{
	    PROTOMSG *ptmp;
	    ptmp = pmsgCur;
	    pmsgCur = pmsgCur->pnext;
	    free(ptmp->pstz);
	    free(ptmp);
	}
#ifdef DEBUG
	unsigned int count = 0;
	PROTOMSG *pmsgCountCur = m_pmsgDbg;
	while(pmsgCountCur != NULL)
	{
	    count++;
	    pmsgCountCur = pmsgCountCur->pnext;
	}
	
	if(count != m_cmsgDbg)
	{
	    fprintf(stderr, "Error: Expected DBG msgs: %d, Actual: %d\n", m_cmsgDbg, count);
	    assert(count == m_cmsgDbg);
	}
#endif
    }
    return TRUE;
}



/* ASCII C String PROTOMSG */
BOOL Protocol::writeMsg(int type, const char *pstz, int msgScope)
{
    int cUTF8ch = 0;
    int idxSrc, idxDst;
    char *pstzUTF8;

    /* GET UTF-8 char COUNT */
    idxSrc = 0;
    while(pstz[idxSrc] != '\0')
    {
	if(pstz[idxSrc] & 0x080)
	    cUTF8ch += 2;
	else
	    cUTF8ch++;
	idxSrc++;
    }

    pstzUTF8 = (char *) malloc(cUTF8ch+1);

    /* DO UTF-8 CONVERSION */
    idxSrc = 0;
    idxDst = 0;
    while(pstz[idxSrc] != '\0')
    {
	if(pstz[idxSrc] & 0x080)
	{
	    pstzUTF8[idxDst++] = (pstz[idxSrc] >> 6) | 0x0C0;	//Set the most significant byte
	    pstzUTF8[idxDst++] = (pstz[idxSrc++] & 0x03F) | 0x080;	//SET LSB
	}
	else	//just copy it
	{
	    pstzUTF8[idxDst++] = pstz[idxSrc++];
	}
    }
    pstzUTF8[idxDst] = '\0';
    return _writeMsg(type, pstzUTF8, msgScope);
}

BOOL Protocol::writeMsgUTF8(int type, const char *pstz, int msgScope)
{
    const char *pcur;
    char *pstzNew;
    int i=1;	//start at 1 so we include the NULL char

    pcur = pstz;
    while(*pcur != '\0')
    {
	i++;
	pcur++;
    }

    pstzNew = (char *) malloc(i * sizeof(char));

    i=0;
    while(TRUE)
    {
	pstzNew[i] = pstz[i];
	if(pstz[i] == '\0')
	    break;
	i++;
    }

    if(!_writeMsg(type, pstzNew, msgScope))
    {
	return FALSE;
    }

    return TRUE;
}

BOOL Protocol::writeMsgUTF16(int type, const XCHAR *pstz, int msgScope)
{
    ConversionResult cr;
    const ConversionFlags cf = lenientConversion;
    int bufSize = 0;
    const XCHAR *pchEnd;
    UTF8 *pstzUTF8;
    UTF8 *pstzUTF8T;

    pchEnd = pstz;

    while(*pchEnd != (XCHAR)'\0')
    {
	pchEnd++;
	bufSize++;
    }
    pchEnd++;
    bufSize++; //count the NULL
    
DO_CONVERSION:
    bufSize *= 2;	//double it
    pstzUTF8T = pstzUTF8 = (UTF8*) malloc(bufSize);
    cr = ConvertUTF16toUTF8((const UTF16**) &pstz, (UTF16*) pchEnd, &pstzUTF8, pstzUTF8 + bufSize, cf);

    if(cr == targetExhausted)
    {
	free(pstzUTF8);
	bufSize *= 2;
	goto DO_CONVERSION;
    }
    else if(cr != conversionOK)
	return FALSE;
    
    if(!_writeMsg(type,(char*) pstzUTF8T, msgScope))
    {
	return FALSE;
    }
    
    return TRUE;
}


BOOL Protocol::_queueMsg(PROTOMSG *pmsg, int msgScope)
{
    switch(msgScope)
    {
	case MSGSCOPE_STANDARD:
	    return _queueMsgStandard(pmsg);
	    break;

	case MSGSCOPE_DEBUG:
	    return _queueMsgDebug(pmsg);
	    break;
    }

    return FALSE;
}

BOOL Protocol::_queueMsgStandard(PROTOMSG *pmsg)
{

    if(m_pmsgLast == NULL)
    {
	assert(m_pmsgLast == m_pmsgTop);
	m_pmsgLast = m_pmsgTop = pmsg;
    }
    else
    {
	m_pmsgLast->pnext = pmsg;
	m_pmsgLast = pmsg;
    }

    m_cmsg++;
    if(m_cmsg > m_cmsgMax)
    {
	return flushBuff();
    }

    return TRUE;
}

int Protocol::flushBuff()
{
    int ret = 0;
    char buf[INT_DEC_SIZE];
    if(m_cmsg > 0)
    {
	Http http(SERVER_HOST);

	//Append Header Info
	http.appendPostData((BYTE*) m_psessionID, m_psessionIDLength);
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));
	http.appendPostData(PROTOCOL_STD_HEADER, strlen(PROTOCOL_STD_HEADER));
	http.appendPostData((BYTE*) &TOK_DELIM, sizeof(TOK_DELIM));

	snprintf(buf, INT_DEC_SIZE, "%u", m_packetCount);
	http.appendPostData((BYTE*) buf, strlen(buf));
	ret =  _flushBuff(&http, &m_pmsgTop);
        if(http.send() == ERR_FATAL)
	    m_fFatalError = true;
	m_packetCount++;
    }
    return ret;
}

int Protocol::_flushBuff(Http *http, PROTOMSG **ppmsgTop)
{
    int cmsg = 0;
    char pstzIdBuf[32];
    char chNull = '\0';
    PROTOMSG *pmsg;
    PROTOMSG *pmsgNext;

    //write PROTOMSGs to protocol
    for(pmsg = *ppmsgTop; pmsg; pmsg = *ppmsgTop)
    {
	pmsgNext = pmsg->pnext;
	//Set type
	http->appendPostData(&TOK_DELIM, sizeof(TOK_DELIM));
	snprintf(pstzIdBuf, 32, "%X", pmsg->type);
	http->appendPostData(pstzIdBuf, strlen(pstzIdBuf)); //ignore NULL terminator
	if(pmsg->size == 0)	//text msg
	{
	    http->appendPostData(&TOK_DELIM, sizeof(TOK_DELIM));
	    http->appendPostData(pmsg->pstz, strlen(pmsg->pstz));	//don't include the NULL terminator
	}
	else	//binary msg
	{
	    http->appendPostData(&TOK_BIN_DELIM, sizeof(TOK_BIN_DELIM));
	    snprintf(pstzIdBuf, 32, "%d", pmsg->size);
	    http->appendPostData(pstzIdBuf, strlen(pstzIdBuf));
	    http->appendPostData(&TOK_DELIM, sizeof(TOK_DELIM));
	    http->appendPostData(pmsg->pb, pmsg->size);
	}
	free(pmsg->pb);
	free(pmsg);
	*ppmsgTop = pmsgNext;
	cmsg++;

	//Update Message Count
	if(ppmsgTop == &m_pmsgTop)
	    m_cmsg--;
	else if(ppmsgTop == &m_pmsgDbg)
	    m_cmsgDbg--;
    }

    http->appendPostData(&chNull, sizeof(chNull)); //NULL terminate the end of the data

    
    //Special MSG queue actions
    if(ppmsgTop == &m_pmsgTop)
    {
	m_pmsgLast = NULL;
	assert(m_cmsg == 0);
	assert(m_pmsgTop == NULL);
    }
    else if(ppmsgTop == &m_pmsgDbg)
    {
	assert(m_pmsgDbg == NULL);
	assert(m_cmsgDbg == 0);
    }

    return cmsg;
}

#ifdef TEST
int main(void)
{
    int i;
    BYTE data[] = {'a', 'b', 'c'};
    Protocol *protocol = new Protocol(5, 5);

    for(i = 0; i < 10; i++)
        protocol->writeMsg(i, "This is a test :) \\ END");

    protocol->writeMsgBin(i++, data, 3);

    protocol->flushBuff();

    delete protocol;
}
#endif

