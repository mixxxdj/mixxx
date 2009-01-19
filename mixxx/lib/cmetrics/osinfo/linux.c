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
#include <stdio.h>
#include "../globaldefs.h"
#include "../utf/utstr.h"
XCHAR *osInfoStz()
{
    XCHAR *buf = malloc(2048);

    // TODO: report kernel version
    FILE *ret;
    if (((ret  = (FILE *) fopen("/proc/version", "r")) == NULL)) {
	// TODO: Read contents of lsb-release
    } else {
    	xsprintf(buf, 500, "Linux kernel verison information unavailable.");
    }
    fclose(ret);

    // TODO: report distro release
    if (((ret  = (FILE *) fopen("/etc/lsb-release", "r")) == NULL)) {
	// TODO: Read contents of lsb-release
    } else {
    	xsprintf(buf, 500, "Non-LSB distro or LSB version info unavailable.");
    }
    fclose(ret);

    return buf;
}
