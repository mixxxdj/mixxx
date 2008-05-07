/**********************************************
 * fsinfo.h - Case Metrics Interface
 *  Copyright 2007 John Sully, Phillip Mendonça-Vieira.
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
 * FSINFO.H - (posix) platform fsinfo contract
 *
 ************************************************/
#include "../globaldefs.h"

#ifndef __FSINFO_H__
#define __FSINFO_H__

#define FSINFO_VERSION 1
#define FSINFO_RECORD_DELIM ';'

#ifdef WIN32
const int CREJECTED_DRIVETYPES = 3;
const long REJECTED_DRIVETYPES[] = {DRIVE_REMOVABLE, DRIVE_NO_ROOT_DIR, DRIVE_CDROM};
#endif //WIN32

// the essential bit. Version 2 or etc ought to include a struct.
// and cross platformness, too
XCHAR *fsInfoStz();

#endif
