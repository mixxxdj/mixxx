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

/************************************************
 * MEMINFO.H - cross platform meminfo struct contract
 *
 ************************************************/

#include "../globaldefs.h"

#ifndef __MEMINFO_H__
#define __MEMINFO_H__

#define MEMINFO_VERSION 1

//We assume long = 4 bytes for x86 and 8 for x64

typedef struct _meminfo
{
    int type_id;
    long memTotal;
    long memFree;
    long swapTotal;
    long swapFree;
    long bufferTotal;
} MEMINFO;

int meminfoGetType();
int meminfoGenStruct(BYTE **pb);

void meminfoParseStruct(BYTE *pb, MEMINFO **ppmemInfo);

XCHAR *memInfoStz();
#endif
