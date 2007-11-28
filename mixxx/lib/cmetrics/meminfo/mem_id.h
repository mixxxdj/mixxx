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

/************************************************
 * MEMID.H - Determins the type_id of the meminfo
 *
 ************************************************/

#ifndef __MEM_ID_H__
#define __MEM_ID_H__

#ifdef __X86__
const int type_id = 0x00; //32-bit address
#elif __X64__
const int type_id = 0x01; //64-bit
#endif

#endif //__MEM_ID_H__
