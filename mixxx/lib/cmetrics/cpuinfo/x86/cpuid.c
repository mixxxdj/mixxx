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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../../globaldefs.h"
#include "cpuid.h"

#ifdef LINUX
#include <unistd.h>
#elif WIN32
#include <windows.h>
#include <intrin.h>
#endif

REGISTERS cpuid(int _eax)
{
    REGISTERS r;
#ifdef __GNUC__ //GCC compiler
#ifdef __X86__
    asm(
           "pushl %%eax     \n\t"
           "pushl %%ebx     \n\t"
           "pushl %%ecx     \n\t"
           "pushl %%edx     \n\t"
           "cpuid           \n\t"
           "movl  %%eax, 0(%%esi)\n\t"
           "movl  %%ebx, 4(%%esi)\n\t"
           "movl  %%ecx, 8(%%esi)\n\t"
           "movl  %%edx, 12(%%esi)\n\t"
           "popl  %%edx     \n\t"
           "popl  %%ecx     \n\t"
           "popl  %%ebx     \n\t"
           "popl  %%eax     \n\t"
            :
            : "a" (_eax),
              "S" (&r)
       );
#elif __X64__
    asm(
            "cpuid"
            : "=a" (r.eax),
              "=b" (r.ebx),
              "=c" (r.ecx),
              "=d" (r.edx)
            : "a" (_eax)
            
        );
#else
#error "Invalid Arch"
#endif
#elif _MSC_VER //VC++ compiler
	int buf[4] = {-1};
	__cpuid(buf, _eax);
	r.eax = buf[0];
	r.ebx = buf[1];
	r.ecx = buf[2];
	r.edx = buf[3];
#else
#error "Unsupported compiler"
#endif

    return r;
}
unsigned int getMaxStandardLvl()
{
    REGISTERS ret;
    ret = cpuid(0x00);
    return ret.eax;
}

unsigned int getMaxExtendedLvl()
{
    REGISTERS ret;
    ret=cpuid(EXTENDED_CPUID_BASE);
    return ret.eax;
}
int cpuidDumpStandard(REGISTERS *rgReg, int cReg)
{
    int i=0;
    int lvlMax = cReg;

    if(cReg > 0)
    {
        rgReg[i] = cpuid(i);
        lvlMax = (rgReg[i].eax < cReg) ? rgReg[i].eax+1 : cReg;
    }
    for(; i<lvlMax; i++)
    {
        rgReg[i] = cpuid(i);
    }
    
    return i;
}

int cpuidDumpExtended(REGISTERS *rgReg, int cReg)
{
    int i=EXTENDED_CPUID_BASE;
    int lvlMax = cReg;

    if(cReg > 0)
    {
        rgReg[0] = cpuid(i);
        lvlMax = (rgReg[0].eax < cReg) ? rgReg[0].eax+1 : cReg;
    }
    for(; i < EXTENDED_CPUID_BASE + lvlMax; i++)
    {
        rgReg[i - EXTENDED_CPUID_BASE] = cpuid(i);
    }
    return i - EXTENDED_CPUID_BASE;
}

#ifdef LINUX
int getNumCores()
{
    return sysconf(_SC_NPROCESSORS_CONF);
}
#elif WIN32
int getNumCores()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return (int) sysInfo.dwNumberOfProcessors;
}
#else
#error No valid OS set
#endif

#ifdef LINUX
long getCpuFreq()
{
    return sysconf(_SC_CLK_TCK);
}
#elif WIN32
long getCpuFreq()
{
    return -1;
}
#else
#error No valid OS set
#endif

char *getVendorID()
{
    char *buf = malloc(13);
    int *st = (int*) buf;
    REGISTERS r;

    r = cpuid(0);
    
    st[0] = r.ebx;
    st[1] = r.edx;
    st[2] = r.ecx;
    buf[12] = '\0';
    return buf;
}

char *getModelName()
{
    int *retBuf = malloc(12 * 4);
    int offset;
    REGISTERS r;

    offset = sizeof(REGISTERS);

    r = cpuid(0x080000002);
    retBuf[0] = r.eax;
    retBuf[1] = r.ebx;
    retBuf[2] = r.ecx;
    retBuf[3] = r.edx;

    r = cpuid(0x080000003);
    retBuf[4] = r.eax;
    retBuf[5] = r.ebx;
    retBuf[6] = r.ecx;
    retBuf[7] = r.edx;

    r = cpuid(0x080000004);
    retBuf[8] = r.eax;
    retBuf[9] = r.ebx;
    retBuf[10] = r.ecx;
    retBuf[11] = r.edx;
    
    return (char*) retBuf;
}

int getFamily()
{
    REGISTERS r;
    int ret;
    r = cpuid(0x001);
    ret = (r.eax & 0x0f00) >> 8;   //get family;
    ret += (r.eax & 0x0ff00000) >> 20; //add extended family
    return ret;
}

int getModel()
{
    REGISTERS r;
    int ret;
    r = cpuid(1);
    
    ret = (r.eax & 0x0f0) >> 4;
    ret += (r.eax & 0x0f0000) >> 12;
    return ret;
}

int getStepping()
{
    REGISTERS r;
    r = cpuid(1);

    return r.eax & 0x0f;
}

long getL1cacheSize()
{
    long ret;
    REGISTERS r;
    r = cpuid(0x080000005);
    ret =  (r.ecx & 0x0ff000000) >> 24;
    ret = ret * 1024;    //output in bytes
    return ret;
}

long getL2cacheSize()
{
    long ret;
    REGISTERS r;
    r = cpuid(0x080000006);

    ret = (r.ecx & 0x0ffff0000) >> 16;
    ret *= 1024;    //adjust to bytes

    return ret;
}

long getL1TLBPageSize()
{
    return 4096;
}

long getL1TLBPages()
{
    long ret;
    REGISTERS r;
    r = cpuid(0x080000005);

    ret = (r.ebx & 0x000ff0000) >> 16;
    return ret;
}

long getL2TLBSize()
{
    long ret4k, ret2M;
    REGISTERS r;
    r = cpuid(0x080000006);

    ret4k = (r.ebx & 0x000ff0000) >> 16; //data TLB
    ret4k += (r.ebx & 0x0000000fff); //code TLB
    ret4k *= 4096;
    
    ret2M = (r.eax & 0x000ff0000) >> 16;
    ret2M += (r.eax & 0x000000fff);
    ret2M *= 2097152;

    return ret4k + ret2M;
}

#ifdef __TEST_CPUID__
#ifdef WIN32
int WinMain(void)
#else
int main(void)
#endif
{
    REGISTERS rgreg[20];
    printf("Standard levels: %d\n", getMaxStandardLvl());
    printf("Extended levels: %u\n", getMaxExtendedLvl()-0x80000000);
    printf("Vendor ID: %s\n", getVendorID());
    printf("Model: %s\n", getModelName());
    printf("Family ID: %d\n", getFamily());
    printf("Model: %d\n", getModel());
    printf("Stepping: %d\n", getStepping());
    printf("L1 Cache: %dK\n", getL1cacheSize()/1024);
    printf("L2 Cache: %dK\n", getL2cacheSize()/1024);
    printf("L1 TLB: %d 4k Pages\n", getL1TLBPages());
    printf("L2 TLB: %d 4k Pages\n", getL2TLBSize()/4096);
}
#endif //__TEST_CPUID__
