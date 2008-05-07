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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "cpuid.h"
#include "cpuinfo.h"
#include "../arch_id.h"
#include "../../globaldefs.h"
#include "../../utf/utstr.h"

int cpuinfoGetType()
{
    return TYPE_ID;
}

int cpuinfoGetStruct(BYTE **ppb)
{
    //This packs the cpuid struct (minus version/size header)
    //care must be taken when porting to different compilers to
    //prevent padding
    
    CPUINFO cpui;
    BYTE *pNextReg;
    int bufsize;

    //Set CPUINFO fields
    cpui.type_id = cpuinfoGetType();
    cpui.total_cores = getNumCores();
    cpui.cStandardLevels = getMaxStandardLvl() + 1;
    cpui.cExtendedLevels = getMaxExtendedLvl() - EXTENDED_CPUID_BASE + 1;

    //Allocate buffer
    bufsize = sizeof(CPUINFO);
    bufsize += cpui.cStandardLevels * sizeof(REGISTERS);
    bufsize += cpui.cExtendedLevels * sizeof(REGISTERS);
    *ppb = (BYTE*) malloc(bufsize);

    //Copy CPUINFO
    memcpy(*ppb, &cpui, sizeof(CPUINFO));

    //Copy the registers
    pNextReg = *ppb + sizeof(cpui);

    cpuidDumpStandard((REGISTERS*) pNextReg, cpui.cStandardLevels);
    pNextReg = pNextReg + sizeof(REGISTERS) * cpui.cStandardLevels;

    cpuidDumpExtended((REGISTERS*) pNextReg, cpui.cExtendedLevels);

    return bufsize;
}

//Sets the pointers to the data within the byte array
void cpuinfoParseStruct(BYTE *pb, 
        CPUINFO **ppcpui, 
        REGISTERS **ppregStandard, 
        REGISTERS **ppregExtended)
{
    *ppcpui = (CPUINFO*) pb;
    *ppregStandard = (REGISTERS*) (pb + sizeof(CPUINFO));
    *ppregExtended = (REGISTERS*) *ppregStandard + (*ppcpui)->cStandardLevels;

}

//Writes a string to be used by protocol
//caller must free string
XCHAR *cpuInfoToStz(BYTE *pCpuInfo)
{
    int bufSize = 0;
    int bufUsed;
    int i;
    XCHAR *buf;
    CPUINFO *pcpui;
    REGISTERS *pregStandard;
    REGISTERS *pregExtended;

    cpuinfoParseStruct(pCpuInfo, &pcpui, &pregStandard, &pregExtended);

    //Calculate max string size
    //+1 is room for delimeter
    bufSize = (UINT_HEX_SIZE + 1) * 4;
    bufSize += (UINT_HEX_SIZE + 1) * pcpui->cStandardLevels * NUM_REGISTERS;    
    bufSize += (UINT_HEX_SIZE + 1) * pcpui->cExtendedLevels * NUM_REGISTERS;

    bufSize++;  //room for NULL
    buf = (XCHAR*) malloc((bufSize) * sizeof(XCHAR));  //alloc string
    //write string
    errno = 0;
    bufUsed = xsprintf(buf, bufSize, "%X|%X|%X|%X", pcpui->type_id, pcpui->total_cores, pcpui->cStandardLevels, pcpui->cExtendedLevels);
    //write standard levels
    for(i = 0; i < pcpui->cStandardLevels; i++)
    {
        bufUsed += xsprintf(buf + bufUsed, bufSize - bufUsed + 1, "|%X|%X|%X|%X",
                pregStandard[i].eax, 
                pregStandard[i].ebx,
                pregStandard[i].ecx,
                pregStandard[i].edx);
    }
    for(i = 0; i < pcpui->cExtendedLevels; i++)
    {
        bufUsed += xsprintf(buf + bufUsed, bufSize - bufUsed + 1, "|%X|%X|%X|%X", 
                pregExtended[i].eax,
                pregExtended[i].ebx,
                pregExtended[i].ecx,
                pregExtended[i].edx);
    }

    assert(errno == 0);
    return buf;
}
#ifdef TEST
#include <stdio.h>
void preg(REGISTERS r)
{
    printf("\tEAX: 0x0%X\n", r.eax);
    printf("\tEBX: 0x0%X\n", r.ebx);
    printf("\tECX: 0x0%X\n", r.ecx);
    printf("\tEDX: 0x0%X\n", r.edx);
}

int main(void)
{
    int size;
    int i;
    BYTE *pb;
    char buf[1000];

    CPUINFO *pcpui;
    REGISTERS *pregStandard;
    REGISTERS *pregExtended;
	
	XCHAR *pstz;

    size = cpuinfoGetStruct(&pb);
    cpuinfoParseStruct(pb, &pcpui, &pregStandard, &pregExtended);


    //Print Fields
    printf("type_id: %d\n", (int) pcpui->type_id);
    printf("total_cores: %d\n", pcpui->total_cores);
    printf("cStandardLevels: %d\n", pcpui->cStandardLevels);
    printf("cExtendedLevels: %d\n", pcpui->cExtendedLevels);
    printf("---------------------\n");
    for(i=0; i<pcpui->cStandardLevels; i++)
    {
        preg(pregStandard[i]);
    }
    printf("----------------------\n");
    for(i=0; i<pcpui->cExtendedLevels; i++)
    {
        preg(pregExtended[i]);
    }

    printf("\n\ncpuInfoToStz:\n");
    pstz = cpuInfoToStz(pb);
    XtoC(pstz, buf);
    printf("\t%s\n", buf);
    free(pstz);

    free(pb);
    return 0;
}
#endif
