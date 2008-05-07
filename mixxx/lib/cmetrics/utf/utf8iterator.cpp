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

#include "../globaldefs.h"
#include "utf8iterator.h"

UTF8Iterator::UTF8Iterator(char *pstz)
{
    m_fSafe = TRUE;	//default to safe
    m_pchFirst = pstz;
    m_pchCur = pstz;
}

UTF8Iterator::UTF8Iterator(char *pstz, BOOL fSafe)
{
    m_fSafe = fSafe;
    m_pchFirst = pstz;
    m_pchCur = pstz;
}

char *UTF8Iterator::pchFirst()
{
    m_pchCur = m_pchFirst;
    return m_pchFirst;
}

char *UTF8Iterator::pchNext()
{
    int cch = 1;
    char chCur;

    if(!m_pchCur)	//DETECT INVAL STZ
	return NULL;

    chCur = *m_pchCur;
    while(chCur & 0x080)
    {
	cch++;
	chCur = chCur << 1;
    }
    
    if(m_fSafe)
    {
	while(cch-- > 0)
	{
	    m_pchCur++;
	    if(m_pchCur == '\0')
		_invalStz();
	}
    }
    else
	m_pchCur = m_pchCur + cch;

    return m_pchCur;
}

char *UTF8Iterator::pchPrev()
{
    if(!m_pchCur)	//DETECT INVAL STZ
	return NULL;

    if(m_pchCur <= m_pchFirst)
    {
	m_pchCur = m_pchFirst;
	return NULL;
    }
    while(fSurrogate(*m_pchCur) && m_pchCur >= m_pchFirst)
	m_pchCur--;
    
    if(m_pchCur < m_pchFirst)
	_invalStz();

    return m_pchCur;
}

void UTF8Iterator::_invalStz()
{
    m_pchFirst = NULL;
    m_pchCur = NULL;
}
