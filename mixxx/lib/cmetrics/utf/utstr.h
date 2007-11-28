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

#ifndef __UTSTR_H__
#define __UTSTR_H__
#include "../globaldefs.h"
/********************************
 * Custom Unicode printf() implementations.
 *
 * xsprintf() takes an ASCII control string
 * xsprintfX() takes a UTF-16 control string
 *
 * Implemented Format Specifiers:
 *      %:  %
 *      c:  ASCII char
 *      C:  UTF-16 char
 *      s:  ASCII string
 *      d or i: signed decimal integer
 *      u:  unsigned decimal integer
 *      x:  unsigned hexadecimal integer
 *      X:  unsigned hexadecimal integer (capitols)
 *      l:  followed by integer conversion type, converts using longs
 *      ll: followed by integer conversion type, converts using 64-bit ints
 *
 * These functions will not write more than bufSize, if we exit
 * prematurely due to this, the function will set errno to -2 (ERR_BUFFOVERFLOW)
 *
 * Return Value: Length of written string
 **********************************/
int xsprintf(XCHAR *buf, unsigned int bufSize, char *ctrlCStr, ...);
int xsprintfX(XCHAR *buf, unsigned int bufSize, XCHAR *ctrlCStr, ...);

/**********************************
 * Integer to String conversion routines.  
 * 
 * WARNING: These do *not* NULL terminate.
 *
 * ltoa():      signed long to string
 * ultoa():     unsigned long to string
 * ultohex():   unsigned long to hex
 *
 **********************************/

int xltoa(int64 n, XCHAR *buf, unsigned int bufSize);
int xultoa(unsigned int64 n, XCHAR *buf, unsigned int bufSize);
int xultohex(unsigned int64 n, int capitol, XCHAR *buf, unsigned int bufSize);

int xstrlen(XCHAR *pstz);
int xstrcat(XCHAR *pstzDst, int dstSize,const XCHAR *pstzSrc);

#ifdef DEBUG
void XtoC(XCHAR *xstr, char *cstr);
#endif
#endif //__UTSTR_H__
