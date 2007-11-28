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

#ifndef __SOCKET_H__
#define __SOCKET_H__
    
typedef struct _state{
        int sockfd;
} SOCKET_STATE;

int sockConnect(SOCKET_STATE *st, const char *addr, int port);
int sockSend(SOCKET_STATE *st, const char *data, unsigned int size);
int sockRecv(SOCKET_STATE *st, void *data, unsigned int max_size);
void sockClose(SOCKET_STATE *st);


#endif //__SOCKET_H__

