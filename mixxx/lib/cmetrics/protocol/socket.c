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
#include "socket.h"
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#pragma comment(lib, "wsock32.lib")
#include <Windows.h>
#include <winsock.h>

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#endif //!WIN32

//Returns < 0 on fail
int sockConnect(SOCKET_STATE *st, const char *addr, int port)
{
    int ret;
    struct hostent *he;
    struct sockaddr_in dest_addr;
    const char *pst = addr;
#ifdef WIN32
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
		return ERR_TEMP;
#endif

    while(*pst != '\0')
        pst++;
    if((he = gethostbyname(addr)) == NULL){
        return ERR_FATAL;
    }
    
    st->sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if(st->sockfd < 0)
        return ERR_TEMP;

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);  //get random port
    dest_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(dest_addr.sin_zero, '\0', sizeof(dest_addr.sin_zero));

    if(dest_addr.sin_addr.s_addr == -1)
        return ERR_FATAL;

    ret = connect(st->sockfd, (struct sockaddr*) &dest_addr, sizeof dest_addr);
    if(ret < 0)
        return ERR_TEMP;

    return OK;
}

int sockSend(SOCKET_STATE *st, const char *data, unsigned int size)
{
    int retSize = 0;
    while(retSize < size)
    {
        retSize = send(st->sockfd, data + retSize, size - retSize, 0);
    }
    return retSize;
}

int sockRecv(SOCKET_STATE *st, void *data, unsigned int max_size)
{
    return recv(st->sockfd, data, max_size, 0);
}

void sockClose(SOCKET_STATE *st)
{
#ifdef WIN32
	closesocket(st->sockfd);
#else
    close(st->sockfd);
#endif
}

#ifdef TEST_SOCKET
int main(int argc, char *argv)
{
    SOCKET_STATE st;
    char buf[0x0ffff];

    sockConnect(&st, "www.rockymedia.ca", 22);
    sockRecv(&st, buf, 0x0ffff);

    printf("%s\n", buf);

    sockClose(&st);
}
#endif
