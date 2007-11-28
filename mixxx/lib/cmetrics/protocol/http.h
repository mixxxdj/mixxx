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

#include "../globaldefs.h"

#ifndef __HTTP_H__
#define __HTTP_H__

#define DEFAULT_DATA_BUFFER 1024
#define DEFAULT_PACKET_BUFFER 2048
#define MAX_HEADER_SIZE 512

#define MAX_URL_SIZE 1024
#define MAX_HOST_SIZE 512

//ERROR CODES
#define OK  0
#define ERRNOMEM -1

/* HTTP HEADER DATA.  THIS MUST BE UTF-8 COMPATIBLE */
#ifdef __HTTP_CPP__
static const char *HTTP_HEADER_POST = "POST \0";
static const char *HTTP_HEADER_HTTP = " HTTP/1.1\0";
static const char *HTTP_HEADER_HOST = "\nHost: \0";
static const char *HTTP_HEADER_USER = "\nUser-Agent: \0";
static const char *HTTP_HEADER_CLENGTH = "\nContent-Length: \0";
static const char *HTTP_HEADER_CTYPE = "\nContent-Type: application/text; charset=UTF-8\0";
static const char *HTTP_HEADER_TAIL = "\n\n\0";
#endif

class Http
{
    public:
        Http(char *host);
        ~Http();
        int appendPostData(const BYTE *data, unsigned int size);
        inline int appendPostData(const char *data, unsigned int size) { return appendPostData((BYTE*) data, size); }
        int send();
        
        char *getResponse();
    private:
        int _openSocket(char *host);
        int _writePacket();
        int _transmit();
        int _parseHTTPResponseCode(char *resp, char **ppstzFail);
        int _parseHTTPContentLength(char *pstz);
        int _setNewLocationFromHTTPLocation(char *pstzResponse);
        int _parseHTTPLocation(char *pstzReponse, char *pstzRetBuf, int retBufLength);
        int _setHostFromURL(const char *pstzURL);

        //TX Packet
        char *m_host;
        char *m_data;
        char *m_packet;
        char *m_cgiPath;
        unsigned int m_port;

        unsigned int m_dataSize;
        unsigned int m_dataUsed;
        unsigned int m_packetUsed;
        unsigned int m_packetSize;

        //RX Packet
        unsigned int m_rxpacketBufSize;    //size of buffer
public: char *m_rxpacket;
        unsigned int m_rxpacketLength;     //length of recieved packet

};

#endif //__HTTP_H__
