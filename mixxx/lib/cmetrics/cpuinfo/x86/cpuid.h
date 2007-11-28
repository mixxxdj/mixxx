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

//x86/64 CPUID interface
#define NUM_REGISTERS 4
typedef struct REGISTERS
{
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
} REGISTERS; 

#define EXTENDED_CPUID_BASE 0x80000000

unsigned int getMaxStandardLvl();
unsigned int getMaxExtendedLvl();

int cpuidDumpStandard(REGISTERS *rgReg, int cReg);
int cpuidDumpExtended(REGISTERS *rgReg, int cReg);
int getNumCores();
long getCpuFreq();
char *getVendorID();
char *getModelName();
int getFamily();
int getModel();
int getStepping();
long getL1cacheSize();
long getL2cacheSize();
long getL1TLBPageSize();
long getL1TLBPages();
long getL2TLBSize();

