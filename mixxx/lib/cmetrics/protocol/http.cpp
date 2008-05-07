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

#define __HTTP_CPP__
#include "http.h"
#undef __HTTP_CPP__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../utf/utf8iterator.h"

#ifdef WIN32
#include <Windows.h>
#define snprintf _snprintf_s
#define sleep Sleep
#else
#include <unistd.h>
#endif

extern "C" {
#include "socket.h"
}

int _istrncpy(char *dest, const char *src, int max)
{
    int c=0;
    while(src[c] != '\0' && c < max)
    {
	dest[c] = src[c];
	c++;
    }
    if(c == max)
	dest[max - 1] = '\0';

    return c;
}

int _ibufncpy(char *dst, const char *src, int cch, int max)
{
	int c;
	int maxCpy = (cch < max) ? cch : max;
	for(c=0; c < maxCpy; c++)
		dst[c] = src[c];
	return c;
}

Http::Http(char *host)
{
    int hostSize;
    int cgiPathSize;
    hostSize = strlen(host) + 1;
    m_host = (char*) malloc(hostSize);
    cgiPathSize = strlen(SERVER_CGI_DIR) + 1;
    m_cgiPath = (char*) malloc(cgiPathSize);

#ifdef WIN32
    strcpy_s(m_host, hostSize, host);
    strcpy_s(m_cgiPath, cgiPathSize, SERVER_CGI_DIR);
#else
    strcpy(m_host, host);
    strcpy(m_cgiPath, SERVER_CGI_DIR);
#endif
    m_port = SERVER_HTTP_PORT;

    m_data = (char*) malloc(DEFAULT_DATA_BUFFER);
    m_dataSize = (m_data) ? DEFAULT_DATA_BUFFER : 0;	//Malloc failure detection
    m_dataUsed = 0;

    m_packet = (char*) malloc(DEFAULT_PACKET_BUFFER);
    m_packetSize = (m_packet) ? DEFAULT_PACKET_BUFFER : 0;

    m_rxpacket = (char*) malloc(DEFAULT_PACKET_BUFFER);
    m_rxpacketBufSize = (m_rxpacket) ? DEFAULT_PACKET_BUFFER : 0;
    m_rxpacketLength = 0;
}

Http::~Http()
{
    free(m_host);
    free(m_cgiPath);
    free(m_data);
    free(m_packet);
    free(m_rxpacket);
}

/************************************
 * parses HTTP responses to get the code, returns the code or -1 on fail
 * *ppstzFail is set to the point of failure, may be used to call iteratively
 * on a buffer with the HTTP response somewhere inside
 *
 * on success *ppstzFail will be set to the next char after the matching code
 * 
 * Perf: since ppstzFail must lie on the start of a UTF-8 char we need
 * 	to be UTF-8 aware here.  Even though its slower
 *
 ************************************/

int Http::_parseHTTPResponseCode(char *pstz, char **ppstzFail)
{
#define BUF_LEN 32
    char pstzHTTP[] = "HTTP/\0";
    char *pchHTTP = pstzHTTP;
    UTF8Iterator chIter(pstz);
    char buf[BUF_LEN];
    int idxBuf = 0;

    //MATCH THE HTTP
    while(*pchHTTP != '\0' && chIter.chCur() != '\0')
    {
	if(chIter.chCur() != *pchHTTP)
	{
	    *ppstzFail = chIter.pchCur();
	    return -1;
	}
	chIter.pchNext();
	pchHTTP++;
    }

    //MATCH A NUMBER
    while(chIter.chCur() != ' ')
    {
	if((chIter.chCur() < '0' || chIter.chCur() > '9' || chIter.chCur() == '\0') && chIter.chCur() != '.')
	{
	    *ppstzFail = chIter.pchCur();
	    return -1;
	}
	chIter.pchNext();
    }
    
    //WHITE SPACE
    while(chIter.chCur() == ' ')
	chIter.pchNext();

    //COPY THE RESPONSE CODE TO THE BUFFER
    while(chIter.chCur() >= '0' && chIter.chCur() <= '9')
    {
	buf[idxBuf++] = chIter.chCur();	//We know this is a valid ASCII char
	if(idxBuf >= BUF_LEN)
	{
	    *ppstzFail = chIter.pchCur();
	    return -1;
	}
	chIter.pchNext();
    }
    buf[idxBuf] = '\0';

    *ppstzFail = chIter.pchCur();
    //TRANSLATE TO INTEGER
    return atoi(buf);
}    

/* Helper Function returns the next line, but NULL if \n\n sequence found */
inline char *pstzNextLine(char *pstz)
{
    char *pch = pstz;
    while(*pch != '\n')
    {
	if(*pch == '\0')
	    return NULL;
	pch++;
    }

    return (*(pch + 1) == '\n') ? NULL : pch + 1;
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

/* Case insensitive character comparison */
inline int chCmpi(char ch1, char ch2)
{
    //Convert to lower case
    if(ch1 >= 'A' && ch1 <= 'Z')
	ch1 = 'a' + (ch1 - 'A');

    if(ch2 >= 'A' && ch2 <= 'Z')
	ch2 = 'a' + (ch2 - 'A');

    return ch1 - ch2;
}

/* Case insensitive version of fsubStr */
inline int fsubStri(const char *pstz1, const char *pstz2)
{
    int i=0;
    while(pstz1[i] != '\0' && pstz2[i] != '\0')
    {
	if(chCmpi(pstz1[i], pstz2[i]) != 0)
	    return FALSE;
	i++;
    }
    return TRUE;
}

int Http::_parseHTTPContentLength(char *pstz)
{
    const char pstzContentLength[] = "Content-Length:\0";
    char *pchCur = pstz;
    char buf[BUF_LEN];
    int idx;
    
    //Find Content-Length field
    while(!fsubStr(pchCur, pstzContentLength))
    {
	if(pchCur == NULL)
	    return 0;
	pchCur = pstzNextLine(pchCur);
    }
    pchCur = pchCur + strlen(pstzContentLength);	//move past header
    
    //Skip White Space
    while(*pchCur == ' ')
	pchCur++;
    
    //Fill buffer with size string
    idx = 0;
    while(*pchCur >= '0' && *pchCur <= '9' && idx < BUF_LEN)
    {
	buf[idx++] = *pchCur;
	pchCur++;
    }
    buf[idx] = '\0';
    
    return atoi(buf);
}

//Returns the Location header field, this will need to be freed by caller
int Http::_parseHTTPLocation(char *pstzHeader, char *pstzOutput, int maxLocationSize)
{
    const char pstzLocation[] = "Location:\0";
    char *pchCur = pstzHeader;
    int i;

    //Find Location field
    while(!fsubStr(pchCur, pstzLocation))
    {
	if(pchCur == NULL)
	    return 0;
	pchCur = pstzNextLine(pchCur);
    }
    pchCur = pchCur + strlen(pstzLocation);

    //Skip White Space
    while(*pchCur == ' ')
	pchCur++;

    //Compute Buffer Size
    for(i = 0; i < (maxLocationSize - 1); i++)
    {
	if(pchCur[i] == '\n' || pchCur[i] == '\0')
	    break;
	pstzOutput[i] = pchCur[i];
    }
    pstzOutput[i] = '\0';
    return i;
}

int Http::_setHostFromURL(const char *pstzURL)
{
    const char *pstzHTTP = "HTTP://\0";
    const char *pchUrlCur = pstzURL;
    char host[MAX_HOST_SIZE];
    char cgiPath[MAX_PATH_SIZE];
    int i;

    //Ignore HTTP header if exists
    if(fsubStri(pstzURL, pstzHTTP))
	pchUrlCur += strlen(pstzHTTP);

    //Parse Host
    i=0;
    while((*pchUrlCur != '/') && (*pchUrlCur != '\0') && i < (MAX_HOST_SIZE - 1))
    {
	host[i++] = *pchUrlCur++;
    }
    host[i] = '\0';

    free(m_host);
    m_host = (char*) malloc(strlen(host) + 1);
    strcpy(m_host, host);

    //Parse path
    i=0;
    while(*pchUrlCur != '\0' && *pchUrlCur != ':' && i < (MAX_PATH_SIZE - 1))
    {
	cgiPath[i++] = *pchUrlCur++;
    }
    cgiPath[i] = '\0';

    free(m_cgiPath);
    m_cgiPath = (char*) malloc(strlen(cgiPath) + 1);
    strcpy(m_cgiPath, cgiPath);

    //Parse port if exists
    if(*pchUrlCur == ':')
    {
	pchUrlCur++;
	m_port = atoi(pchUrlCur);
    }

    return true;
}


int Http::appendPostData(const BYTE *data, unsigned int size)
{

WRITE_DATA:
    if(size > (m_dataSize - m_dataUsed))
	goto DOUBLE_BUFFER;

    memcpy(m_data + m_dataUsed, data, size);
    m_dataUsed += size;

    return OK;

DOUBLE_BUFFER:
    m_dataSize *= 2;
    m_data = (char*) realloc(m_data, m_dataSize);
    if(!m_data)
	return ERRNOMEM;
    goto WRITE_DATA;
}

int Http::send()
{
    int result;
    
    result = _writePacket();
    if(result == OK)
	result = _transmit();

    return result;
}

int Http::_openSocket(char *host)
{
    //Socket Code goes here
	return 0;
}

int Http::_writePacket()
{
#define STRBUFLEN 32
    unsigned int written = 0;
    char intToStrBuf[STRBUFLEN];
    char *pstSlash = "/";

//Macro to make putting the packet together easily.  Automatically doubles the buffer and restarts
//the packet building process if out of buffer space
#define append(src) written += _istrncpy(m_packet + written, src, m_packetSize - written); if(written == m_packetSize) goto DOUBLE_BUFFER
    
    //Check Packet Buffer
    if((m_dataSize + MAX_HEADER_SIZE) > m_packetSize)
    {
	free(m_packet);
	m_packetSize = m_dataSize + MAX_HEADER_SIZE;
	m_packet = (char*) malloc(m_packetSize);
	if(!m_packet)
	    return ERRNOMEM;
    }
    
    //Write Header Fields
WRITE_PACKET:
    written = 0;
    //POST
    append(HTTP_HEADER_POST);
    append(m_cgiPath);
    append(HTTP_HEADER_HTTP);
    //HOST
    append(HTTP_HEADER_HOST);
    append(m_host);
    //USER AGENT
    append(HTTP_HEADER_USER);
    append(LIB_NAME);
    append(pstSlash);
    append(LIB_VERSION);
    //CONTENT LENGTH
    append(HTTP_HEADER_CLENGTH);
    snprintf(intToStrBuf, STRBUFLEN, "%d", m_dataUsed);
    append(intToStrBuf);
    //CONTENT TYPE
    append(HTTP_HEADER_CTYPE);
    //TAIL
    append(HTTP_HEADER_TAIL);
    //BODY
	written += _ibufncpy(m_packet + written, m_data, m_dataUsed, m_packetSize - written); if(written == m_packetSize) goto DOUBLE_BUFFER;
    //TRAILING NULLS
    append(HTTP_HEADER_TAIL);

    m_packetUsed = written;
    return OK;

DOUBLE_BUFFER:
    //Called if we run out of buffer space, we then rewrite the packet from scratch (try not to go here)
    delete m_packet;
    m_packetSize *= 2;
    m_packet = new char[m_packetSize];
    if(!m_packetSize)
	return ERRNOMEM;
    goto WRITE_PACKET;

#undef STRBUFLEN
#undef append
}

char *recvHTTPHeader(SOCKET_STATE *pss)
{
#define BUF_SIZE 0x0ffff	//headers longer than this are unsupported
    int headerSize = 0;
    char recvBuf[BUF_SIZE];
    int flagOK = FALSE;

    char *header = NULL;
    
    do{
	sockRecv(pss, (void*) (recvBuf + headerSize), 1);
	headerSize++;
	if(headerSize >= 4)
	{
	    if((recvBuf[headerSize - 4] == '\r') 
		&& (recvBuf[headerSize - 3] == '\n')
		&& (recvBuf[headerSize - 2] == '\r')
		&& (recvBuf[headerSize - 1] == '\n'))
	    {
		flagOK = TRUE;
		break;
	    }
	}
    }while(headerSize < BUF_SIZE);

    if(flagOK)
    {
	header = (char*)malloc(headerSize);
	strncpy(header, recvBuf, headerSize);
	header[headerSize-1] = '\0';
    }

    return header;
#undef BUF_SIZE
}

/**************************
 * Main data transfer routine, sends packet over INET and recieves response
 *
 * Side Effects: m_rxpacket is a valid server response when returning OK.
 * 			is valid until next _transmit()
 *
 *************************/

int Http::_transmit()
{
    SOCKET_STATE st;

    int ret;
    int cTrys = 0;
    int cHTTPRetry = 0;
    char bufURL[MAX_URL_SIZE];

    //HTTP returns
    char *pheader, *pheaderStart;
    int responseCode;
    unsigned int contentLength;

#ifdef DEBUG
    printf("TRANSMITTING...\n");
#endif
RESTART:
    if(cHTTPRetry > SERVER_MAX_RETRY_C)
	return ERR_TEMP;

    cTrys = 0;
    do{	//we loop until we get a fatal error or a connection
	cTrys++;
	if(cTrys > MAX_CONNECT_ATTEMP)
	    return ERR_FATAL;

	ret = sockConnect(&st, m_host, m_port);
	if(ret == OK)
	    break;
	else if(ret == ERR_TEMP)
	    sleep(SERVER_RETRY_T);			//wait a bit then try
	else
	    return ret;
    } while(1);

    //Recieve HTTP status
    sockSend(&st, m_packet, m_packetUsed);
#ifdef DEBUG
#ifdef WIN32
	MessageBox(NULL, m_packet, "Transmitted", NULL);
#endif
#endif
    pheaderStart = pheader = recvHTTPHeader(&st);

    //Parse Response Code
    do{
	responseCode = _parseHTTPResponseCode(pheaderStart, &pheaderStart);
	if(pheaderStart == NULL)
	{
	    free(pheader);
	    sockClose(&st);
	    return ERR_FATAL;
	}
    }while(responseCode < 0 && *pheaderStart == '\0');


    //HANDLE KNOWN CODES
    switch(responseCode)
    {
	//NOT MENTIONED CODES CONSIDRED FATAL ERRORS
	case 100:
	case 200:
	case 203:
	case 204:
	case 205:
	case 206:
	    break;	//consider success

	//case 300:
	case 301:	//Permanent Redirect
	    sockClose(&st);
	    cHTTPRetry++;
	    if(_parseHTTPLocation(pheaderStart, bufURL, MAX_URL_SIZE) == 0)
		return ERR_FATAL;
		free(pheader);
	    _setHostFromURL(bufURL);
	    _writePacket();
#ifdef DEBUG
	    fprintf(stderr, "Permanently changing host to: %s\n", m_host);
	    fprintf(stderr, "Permanently changing path to: %s\n", m_cgiPath);
	    fprintf(stderr, "Permanently changing port to: %d\n", m_port);
#endif
	    goto RESTART;

	case 408:
	case 500:	
	case 504:	//retry from start
	    sockClose(&st);
	    free(pheader);
	    cHTTPRetry++;
	    sleep(SERVER_RETRY_T);
	    goto RESTART;
	
	case 503:	//wait extra long
	    sockClose(&st);
	    free(pheader);
	    cHTTPRetry+=2;	//we don't retry as often in this case
	    sleep(SERVER_RETRY_T * 2);
	    goto RESTART;

	default:
	    free(pheader);
	    sockClose(&st);
	    return ERR_FATAL;
    }  
    //Fall through means everything is ok 
    
    //Recv the content
    contentLength = _parseHTTPContentLength(pheaderStart);	
    if(m_rxpacketBufSize < contentLength)
    {
	free(m_rxpacket);
	m_rxpacketBufSize = contentLength + 1; //+1 accounts for added '\0'
	m_rxpacket = (char*) malloc(m_rxpacketBufSize);
    }
    m_rxpacketLength = sockRecv(&st, (void*)m_rxpacket, contentLength);
    m_rxpacket[m_rxpacketLength] = '\0'; 

    free(pheader);
    sockClose(&st);

#ifdef DEBUG
    m_packet[m_packetUsed] = '\0';
    printf("PACKET USED: %d\n", m_packetUsed);
    printf("PACKET: \n---------------\n%s\n", m_packet);
    printf("RESPONSE: %s\n", m_rxpacket);
    printf("RESPONSE CODE: %d\n", responseCode);
    printf("RESPONSE LENGTH: %d\n", contentLength);
    printf("===================\n");
#ifdef WIN32
    MessageBox(NULL, m_rxpacket, "Response", NULL);
#endif
#endif
    return OK;
}

#ifdef TEST_HTTP
int main(void)
{
    Http http("www.rockymedia.ca");
    http.appendPostData("This is a test of the post data", 32);
    http.send();
}
#endif //TEST
