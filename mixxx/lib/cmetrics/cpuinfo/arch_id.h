/**********************************************
 * arch_id.h - Case Metrics Interface
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
 * ARCH_ID.H - defines a numeric type id for each supported architecture
 *
 ************************************************/
#ifndef __ARCH_ID_H__
#define __ARCH_ID_H__

#ifdef __X86__
#define TYPE_ID 0x00;
#elif __X64__
#define TYPE_ID 0x01;
#endif
//Future Arch IDs follow here



#endif //__ARCH+ID_H__
