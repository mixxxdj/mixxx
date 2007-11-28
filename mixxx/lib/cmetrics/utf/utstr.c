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

#include "../globaldefs.h"
#include "utstr.h"
#include <assert.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>

//CONSTANTS
#define INT_CHARS 11 //max chars in an integer to string conversion
#define LINT_CHARS 20
#define LINT_HEX_CHARS 16
const int ERR_BUFFOVERFLOW = -2;
const XCHAR XCHAR_NULL = (XCHAR) '\0';

//ERROR
int _errnoUTSTR;

int xstrlen(XCHAR *pstz)
{
    int c=0;
    while(pstz[c] != XCHAR_NULL)
        c++;
    return c;
}

int xstrcat(XCHAR *pstzDst, int dstSize,const XCHAR *pstzSrc)
{
    int idxSrc=0, idxDst = 0;
    dstSize--;  //This will make sure theres always room for '\0'
    
    while(pstzDst[idxDst] != '\0')
    {
        idxDst++;      //go to end of dest
        if(idxDst >= dstSize)
            goto OVERFLOW;
    }
    while(pstzSrc[idxSrc] != '\0')
    {
        pstzDst[idxDst++] = pstzSrc[idxSrc++];
        if(idxDst >= dstSize)
            goto OVERFLOW;
    }

OVERFLOW:
    pstzDst[idxDst] = '\0';
    return idxDst;
}

/* HELPER FUNCTIONS */

#define idxInc(x) if(++x >= bufSize) goto OVERFLOW

int ultohex(unsigned int64 n, int capitol, XCHAR *buf, unsigned int bufSize)
{
    int idx = 0;
    int idxT = 0;
    int strLen;
    char bufT[LINT_HEX_CHARS];
    
    if(bufSize < 1)
        goto OVERFLOW;

    //Generate digits in reverse
    do{
        bufT[idxT++] = ((n %16) < 10) ? 
            (n %16) + '0' 
            : (n %16) - 10 + ((capitol) ? 'A' : 'a');
    }while((n /= 16) > 0);

    //reverse and pass to output buffer
    strLen = idxT;
    idxT--;
    while(idx < strLen)
    {
        buf[idx] = (XCHAR) bufT[idxT--];
        idxInc(idx);
    }
    return idx;

OVERFLOW:
    _errnoUTSTR = ERR_BUFFOVERFLOW;
    return bufSize;
}


//won't overflow bufSize, no null terminator
//returns number of chars written
int uxltoa(unsigned int64 n, XCHAR *buf, unsigned int bufSize)
{
    int idx = 0;
    int idxT = 0;
    int strLen;
    char bufT[LINT_CHARS];
    if(bufSize < 1)
        goto OVERFLOW;

    //Generate digits in reverse
    do{
        bufT[idxT++] = n %10 + '0';
    } while((n /= 10) > 0);

    //reverse and pass to output buffer
    strLen = idxT;
    idxT--;
    while(idx < strLen)
    {
        buf[idx] = (XCHAR) bufT[idxT--];
        idxInc(idx);
    }

    return idx;

OVERFLOW:
    _errnoUTSTR = ERR_BUFFOVERFLOW;
    return bufSize;
}
int xltoa(int64 n, XCHAR *buf, unsigned int bufSize)
{
    int idx = 0;
    if(bufSize < 1)
        goto OVERFLOW;

    if(n < 0)
    {
        buf[idx] = '-';
        idxInc(idx);
        if(n == ((int64) -LONG_LONG_MAX - 1)) //weird number
        {
            //We know what it is so just set it statically and exit
            if(sizeof(int64) == 8)
            {
                buf[idx] = '9'; idxInc(idx);
    
                buf[idx] = '2'; idxInc(idx);
                buf[idx] = '2'; idxInc(idx);
                buf[idx] = '3'; idxInc(idx);

                buf[idx] = '3'; idxInc(idx);
                buf[idx] = '7'; idxInc(idx);
                buf[idx] = '2'; idxInc(idx);

                buf[idx] = '0'; idxInc(idx);
                buf[idx] = '3'; idxInc(idx);
                buf[idx] = '6'; idxInc(idx);
            
                buf[idx] = '8'; idxInc(idx);
                buf[idx] = '5'; idxInc(idx);
                buf[idx] = '4'; idxInc(idx);
            
                buf[idx] = '7'; idxInc(idx);
                buf[idx] = '7'; idxInc(idx);
                buf[idx] = '5'; idxInc(idx);
            
                buf[idx] = '8'; idxInc(idx);
                buf[idx] = '0'; idxInc(idx);
                buf[idx] = '8'; idxInc(idx);
            }
            else if(sizeof(int64) == 4)
            {
				assert(FALSE);
                buf[idx] = '2'; idxInc(idx);
                
                buf[idx] = '1'; idxInc(idx);
                buf[idx] = '4'; idxInc(idx);
                buf[idx] = '7'; idxInc(idx);

                buf[idx] = '4'; idxInc(idx);
                buf[idx] = '8'; idxInc(idx);
                buf[idx] = '3'; idxInc(idx);

                buf[idx] = '6'; idxInc(idx);
                buf[idx] = '4'; idxInc(idx);
                buf[idx] = '8'; idxInc(idx);
            }
            return idx;
        }
        n *= -1;    //make positive
    }
    _errnoUTSTR = 0;
    idx += uxltoa((unsigned long) n, buf + idx, bufSize - idx);
  
    return idx;

OVERFLOW:
    _errnoUTSTR = ERR_BUFFOVERFLOW;
    return bufSize;
} 

/********************************
 * Custom Unicode printf() implementations.
 *
 * xsprintfC() takes an ASCII control string
 * xsprintfX() takes a UTF-16 control string
 *
 * NOTE: format specifiers can be added as needed.
 *
 * UPDATING: Ensure standard printf() format specifiers behave the same
 *      and add any relevant tests.  Keep these comments in sync with the
 *      the header file.
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
 *		ll: followed by integer conversion type, converts using 64-bit integers
 *
 * These functions will not write more than bufSize, if we exit
 * prematurely due to this, the function will set errno to -2 (ERR_BUFFOVERFLOW)
 *
 * Return Value: Length of written string
 **********************************/


int xsprintf(XCHAR *buf, unsigned int bufSize, char *ctrlCStr, ...)
{
    int ctrlIdx = 0;
    int outputIdx = 0;

    //scratch vars
    int itemp;
    unsigned int utemp;
    long ltemp;
    unsigned long ultemp;
	int64 lltemp;
	unsigned int64 ulltemp;
    int flag = 0;

    char *pstz;
    va_list list;

    va_start(list, ctrlCStr);


    while(ctrlCStr[ctrlIdx] != '\0')
    {
        if(ctrlCStr[ctrlIdx] == '%')
        {
            ctrlIdx++;
            switch(ctrlCStr[ctrlIdx])
            {
                case '%': //% symbol
                    buf[outputIdx] = (XCHAR) '%';
                    idxInc(outputIdx);
                    break;

                case 'c': //ASCII char
                    buf[outputIdx] = (XCHAR) va_arg(list, int); //char promoted to int
                    idxInc(outputIdx);
                    break;

                case 'C': //UTF-16 char
                    buf[outputIdx] = va_arg(list, int); //XCHAR promoted to int
                    idxInc(outputIdx);
                    break;
                
                case 's': //ASCII string
                    pstz = va_arg(list, char*);
                    while (*pstz != '\0')
                    {
                        buf[outputIdx] = (XCHAR)*pstz;
                        pstz++;
                        idxInc(outputIdx);
                    }
                    break; 


                case 'd':
                case 'i':   //signed decimal
                    itemp = va_arg(list, int);
                    _errnoUTSTR = 0;
                    outputIdx += xltoa(itemp, buf + outputIdx, bufSize - outputIdx);
                    if(_errnoUTSTR != 0)
                        goto OVERFLOW;
                    break;

                case 'u':   //unsigned decimal
                    utemp = (unsigned int) va_arg(list, unsigned int);
                    _errnoUTSTR = 0;
                    outputIdx += uxltoa((unsigned long) utemp, buf + outputIdx, bufSize - outputIdx);
                    if(_errnoUTSTR != 0)
                        goto OVERFLOW;
                    break;

                case 'X':   flag = 1; //cpaitol hex
                case 'x':   //lowercase hex
                    utemp = va_arg(list, unsigned int);
                    _errnoUTSTR = 0;
                    outputIdx += ultohex((unsigned long)utemp, flag, buf + outputIdx, bufSize - outputIdx);
                    flag = 0;
                    if(_errnoUTSTR != 0)
                        goto OVERFLOW;
                    break;

                case 'l':   //long
                    ctrlIdx++;
                    switch(ctrlCStr[ctrlIdx])
                    {
						case 'l':
							ctrlIdx++;
							switch(ctrlCStr[ctrlIdx])
							{
								/* long long (64-bit) */
								case 'd':
								case 'i':
									lltemp = va_arg(list, int64);
									_errnoUTSTR = 0;
									outputIdx += xltoa(lltemp, buf + outputIdx, bufSize - outputIdx);
									if(_errnoUTSTR != 0)
										goto OVERFLOW;
									break;

								case 'u':
									ulltemp = va_arg(list, unsigned int64);
									_errnoUTSTR = 0;
									outputIdx += uxltoa(ulltemp, buf + outputIdx, bufSize - outputIdx);
									if(_errnoUTSTR != 0)
										goto OVERFLOW;
									break;

								case 'X':   flag = 1; //capitol hex
								case 'x':   //lowercase hex
									ulltemp = va_arg(list, unsigned int64);
									_errnoUTSTR = 0;
									outputIdx += ultohex(ulltemp, flag, buf + outputIdx, bufSize - outputIdx);
									flag = 0;
									if(_errnoUTSTR != 0)
										goto OVERFLOW;
									break;

								default:
									errno = -1;
									buf[outputIdx] = ctrlCStr[ctrlIdx++];
									idxInc(outputIdx);
							}
							break;

						/* long */
                        case 'd':
                        case 'i':
                            ltemp = va_arg(list, long);
                            _errnoUTSTR = 0;
                            outputIdx += xltoa(ltemp, buf + outputIdx, bufSize - outputIdx);
                            if(_errnoUTSTR != 0)
                                goto OVERFLOW;
                            break;

                        case 'u':
                            ultemp = va_arg(list, unsigned long);
                            _errnoUTSTR = 0;
                            outputIdx += uxltoa(ultemp, buf + outputIdx, bufSize - outputIdx);
                            if(_errnoUTSTR != 0)
                                goto OVERFLOW;
                            break;

                        case 'X':   flag = 1; //capitol hex
                        case 'x':   //lowercase hex
                            ultemp = va_arg(list, unsigned long);
                            _errnoUTSTR = 0;
                            outputIdx += ultohex(ultemp, flag, buf + outputIdx, bufSize - outputIdx);
                            flag = 0;
                            if(_errnoUTSTR != 0)
                                goto OVERFLOW;
                            break;

                        default:
                            errno = -1;
                            buf[outputIdx] = ctrlCStr[ctrlIdx++];
                            idxInc(outputIdx);
                    }
                    break;

                case '\0':
                    buf[outputIdx] = '\0';
                    idxInc(outputIdx);
                    break;
                case ' ':
                    break;

                default: //invalid
                    errno = -1;
                    buf[outputIdx] = ctrlCStr[ctrlIdx - 1]; idxInc(outputIdx); //show the %
                    buf[outputIdx] = ctrlCStr[ctrlIdx];   idxInc(outputIdx);
            }
        }
        else
        {
            buf[outputIdx] = (XCHAR) ctrlCStr[ctrlIdx];
            idxInc(outputIdx);
        }
        ctrlIdx++;
    }

    buf[outputIdx] = '\0';
    //if(xstrlen(buf) != outputIdx)
    //    assert(xstrlen(buf) == outputIdx);
    //outputIdx++;
    return outputIdx;

    //ERROR HANDLING
OVERFLOW:
    errno = ERR_BUFFOVERFLOW;
    buf[bufSize - 1] = '\0';
    return (bufSize - 1);
}

#ifdef DEBUG
void XtoC(XCHAR *xstr, char *cstr)
{
    int i;
    i=0;
    while(xstr[i] != '\0')
    {
        cstr[i] = (char) xstr[i];
        i++;
    }
    cstr[i] = (char) xstr[i];
#ifdef LIB_TEST
    cstr[i+1] = (char) xstr[i + 1];
#endif
}
#endif //DEBUG


#ifdef LIB_TEST
#include "stdio.h"
#include "string.h"
#include <assert.h>
//#include "ConvertUtf.h"
int main(void)
{
    XCHAR Xbuf[0x0ffff] = {'.'};
    XCHAR XbufLengthTest[0x0ffff] = {'.'};
    char cbuf[0x0ffff] = {'.'};
    char pfbuf[0x0ffff] = {'.'};
    char cbufLengthTest[0x0ffff] = {'.'};
    int i;
    int length;

    char carg = 'a';
    int iarg = -1234567890;
    int izero = 0;
    unsigned int uarg = 1234567890;
    //int iweird = -2147483648;
    int iweird = (-INT_MAX - 1);
    //int ihex = 0x0F00BA3;
    //int ihex = 0x080000018;
    int ihex = 0x080000000;
    char pstz[] = "this is a test, bitch";
    long larg = -1234567890;
    long lzero = 0;
    unsigned long ularg = 1234567890;
    long lweird = (-LONG_MAX - 1);
    long lhex = 0x01234567890F00B;

    for(i=0; i<0x0ffff; i++)
    {
        Xbuf[i] = XbufLengthTest[i] = (XCHAR) '.';
        cbuf[i] = pfbuf[i] = cbufLengthTest[i] = '.';
    }
    
    //test sprintf() compatibility
    printf("sprintf() compatibility: ");
    
    char printf_format[] =  "%%, %c, %i, %i, %i, %u, %ld, %li, %li, %lu, %x, %X, %lx, %lX, %s % %r\n%\0";
    xsprintf(Xbuf, 0x0ffff, printf_format, carg, iarg, iweird, izero, uarg, larg, lweird, lzero, ularg, ihex, ihex, lhex, lhex, pstz);
    sprintf(pfbuf,          printf_format, carg, iarg, iweird, izero, uarg, larg, lweird, lzero, ularg, ihex, ihex, lhex, lhex, pstz);
    XtoC(Xbuf, cbuf);
    
    length = strlen(cbuf);

    if(strcmp(cbuf, pfbuf) == 0)
        printf("OK\n");
    else
    {
        printf("FAIL\n");
        printf("\tformat:   %s\n", printf_format);
        printf("\txsprintf: %s\\0\n", cbuf);
        printf("\tsprintf:  %s\\0\n", pfbuf);
        return -1;
    }

    //Print output
    printf("New Formats test: ");
    xsprintf(Xbuf, 0x0ffff, "%C", 0x04F23);
    if(Xbuf[0] == 0x04F23 && Xbuf[1] == '\0')
        printf("OK\n");
    else
        printf("FAIL\n");
 

    //test length handling
    printf("Buffer overrun handling test: ");

    for(i=1; i <= length; i++)
    {
        assert(cbufLengthTest[i] == '.');
        xsprintf(XbufLengthTest, i, printf_format, carg, iarg, iweird, izero, uarg, larg, lweird, lzero, ularg, ihex, ihex, lhex, lhex);
        XtoC(XbufLengthTest, cbufLengthTest);
        if(xstrlen(XbufLengthTest) != (i-1))
                printf("\n\tFAIL: %d chars, expected: %d\n", strlen(cbufLengthTest), i-1);
        else if(strlen(cbufLengthTest) == (i-1) && XbufLengthTest[i] == '.')
            ;
        else
        {
            if(XbufLengthTest[i] != '.')
                printf("\n\tFAIL: buffer overwritten on length+1: '%c', expected: '%c'\n", cbufLengthTest[i], '.');
            printf("\t\tString: %s\\0\n", cbufLengthTest);

            if(i < length && errno != ERR_BUFFOVERFLOW)
                printf("\n\tFAIL: Error code not set\n");

            return -1;
        }

    }
    printf("OK\n");
    
    //Return size test
    printf("Retrun length test: ");
    char printf_formatl[] = "%%, %c, %i, %i, %i, %u, %ld, %li, %li, %lu, %x, %X, %lx, %lX, % %r\n%%  % %\0";
    length = xsprintf(Xbuf, 0x0ffff, printf_formatl, carg, iarg, iweird, izero, uarg, larg, lweird, lzero, ularg, ihex, ihex, lhex, lhex);
    if(length == xstrlen(Xbuf)+1)
        printf("OK\n");
    else
    {
        printf("FAIL\n\tExpected: %d\n\tReturned: %d\n", xstrlen(Xbuf)+1, length);
        return -1;
    }
    
    //xstrcat test
    int ret;
    printf("xstrcat() overflow test: ");
    XCHAR Xbuf2[512];
    xsprintf(Xbuf, 512, "This string is 24 chars");
    Xbuf2[0] = '\0';
    xstrcat(Xbuf2, 3, Xbuf);
    if( (Xbuf2[1] == 'h') &&
        (Xbuf2[2] == '\0') &&
        xstrlen(Xbuf2) == 2)
        printf("OK\n");
    else
    {
        printf("FAIL\n");
    }

    return 0;
}

#endif //TEST
