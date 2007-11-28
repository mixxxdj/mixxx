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

/*****************************************
 * win32.c: OSinfo module for windows
 *
 *****************************************/

#ifndef WIN32
#error "Invalid operating system"
#endif

#include "osinfo.h"
#include "../globaldefs.h"
#include <windows.h>

XCHAR *osInfoStz()
{
	OSVERSIONINFOEX osvi;
	int cch;
	XCHAR *pstz;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO*) &osvi);
	
	cch = (UINT_HEX_SIZE + 1) * 7; /* +1 is room for deliminator */
	cch++;	//room for NULL
	pstz = malloc(cch * sizeof(XCHAR));

	//Format: MAJOR_VERSION|MINOR_VERSION|BUILD#|SERVICEPACK_MAJOR|SERVICEPACK_MINOR|SUITE_MASK|PRODUCT_TYPE
	xsprintf(pstz, cch, "%X|%X|%X|%X|%X|%X|%X", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber, (int) osvi.wServicePackMajor, (int) osvi.wServicePackMinor, (int) osvi.wSuiteMask, (int) osvi.wProductType);

	return pstz;
}

#ifdef TEST
int main(void)
{
	XCHAR *pstz;
	char buf[512];
	pstz = osInfoStz();
	
	XtoC(pstz, buf);
	printf("%s\n", buf);
	free(pstz);
}
#endif