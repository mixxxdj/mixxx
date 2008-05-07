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

#include <assert.h>
#include <stdlib.h>

#include "meminfo.h"
#include "mem_id.h"
#include "../utf/utstr.h"

#ifdef LINUX
#include <sys/sysinfo.h>
#elif WIN32
#include <windows.h>
#endif

int meminfoGetType()
{
    return type_id;
}

int meminfoGenStruct(BYTE **pb)
{
    MEMINFO *pmemInfo;
#ifdef LINUX
    struct sysinfo psysInfo;
#elif WIN32
    MEMORYSTATUSEX memStatEx;
    MEMORYSTATUS memStat;
	OSVERSIONINFO osvi;
	long swapUsed;
	int fWIN2K;
#endif

    //Verify Build Sanity
    #ifdef __X86__
    assert(sizeof(long) == 4);
    #elif __X64__
    assert(sizeof(long) == 8);
    #endif
    
    pmemInfo = malloc(sizeof(MEMINFO));
    
    //Get Data from OS
#ifdef LINUX
    if(sysinfo(&psysInfo) == -1)
    {
        free(pmemInfo);
        return -1;
    }
#elif WIN32
	/* Detect Windows Version */
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	fWIN2K = osvi.dwMajorVersion >= 5;	//Set if WIN 2000 or later
    if(fWIN2K)
    {
        memStatEx.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memStatEx);
    }
    else
    {
        memStat.dwLength = sizeof(MEMORYSTATUS);
        GlobalMemoryStatus(&memStat);
    }
#endif
    
    //Fill struct
    pmemInfo->type_id = meminfoGetType();

#ifdef LINUX
    pmemInfo->memTotal = psysInfo.totalram;
    pmemInfo->memFree = psysInfo.freeram;
    pmemInfo->swapTotal = psysInfo.totalswap;
    pmemInfo->swapFree = psysInfo.freeswap;
    pmemInfo->bufferTotal = psysInfo.bufferram;
#elif WIN32
    if(fWIN2K) //Windows 2000 or later
    {
        pmemInfo->memTotal = memStatEx.ullTotalPhys;
        pmemInfo->memFree = memStatEx.ullAvailPhys;
        pmemInfo->swapTotal = memStatEx.ullTotalPageFile - memStatEx.ullTotalPhys;
        pmemInfo->swapFree = (long) -1;
        pmemInfo->bufferTotal = (long) -1;
    }
    else //WIN ME or previous
    {
        pmemInfo->memTotal = memStat.dwTotalPhys;
        pmemInfo->memFree = memStat.dwAvailPhys;
        pmemInfo->swapTotal = memStat.dwTotalPageFile - memStat.dwTotalPhys;
        pmemInfo->swapFree = (long) -1;
        pmemInfo->bufferTotal = (long) -1;
    }
#endif
    *pb = (BYTE*) pmemInfo;
    return(sizeof(MEMINFO));
}

//MEMINFO pointer points to data in pb
void meminfoParseStruct(BYTE *pb, MEMINFO **ppmemInfo)
{
    *ppmemInfo = (MEMINFO*) pb;
}

XCHAR *memInfoStz()
{
    XCHAR *buf;
    int bufSize;
    BYTE *pb = NULL;
    MEMINFO *pmemInfo;

    meminfoGenStruct(&pb);
    meminfoParseStruct(pb, &pmemInfo);

    //calculate bufsize
    bufSize = ULONG_HEX_SIZE * 5;
    bufSize += 1;

    buf = malloc(bufSize * sizeof(XCHAR));

    xsprintf(buf, bufSize, "%X|%X|%X|%X|%X", pmemInfo->memTotal, pmemInfo-> memFree, pmemInfo->swapTotal, pmemInfo->swapFree, pmemInfo->bufferTotal);
    
    free(pb);
    return buf;
}
 

#ifdef TEST
#include <stdio.h>
int main(void)
{
    BYTE *pb;
    MEMINFO *pmemInfo;
    XCHAR *pstzProtocol;
    char buf[1024];

    meminfoGenStruct(&pb);
    meminfoParseStruct(pb, &pmemInfo);
    

    printf("type_id: %d\n", pmemInfo->type_id);
    printf("memTotal: %ld\n", pmemInfo->memTotal);
    printf("memFree: %ld\n", pmemInfo->memFree);
    printf("swapTotal: %ld\n", pmemInfo->swapTotal);
    printf("bufferTotal: %ld\n", pmemInfo->bufferTotal);

    pstzProtocol = memInfoStz();
    XtoC(pstzProtocol, buf);
    printf("PROTOCOL STRING: %s\n", buf);
    free(pstzProtocol);
    free(pb);
    return 0;
}
#endif //TEST
