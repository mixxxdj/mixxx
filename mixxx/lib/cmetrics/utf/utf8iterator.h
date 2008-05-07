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

/**********************************
 *  UTF8Iterator: iterates through a UTF8 string
 *      
 *  Will return all NULLs after reaching an invalid char
 *
 ***********************************/

#define fSurrogate(ch) ((ch & 0x080) && ((~ch) & 0x040))

class UTF8Iterator
{
public:
    UTF8Iterator(char *pstz);
    UTF8Iterator(char *pstz, BOOL fSafe);
    char *pchFirst();
    char *pchNext();
    char *pchPrev();
    char *pchCur() { return m_pchCur; };
    char chCur() { return (m_pchCur) ? *m_pchCur : '\0'; };

private:
    void _invalStz();   //invalid string handler

    BOOL m_fSafe;       //if Check the string for validitiy
    //Always keep these on the beginning of a code point
    char *m_pchFirst;
    char *m_pchCur;
};
