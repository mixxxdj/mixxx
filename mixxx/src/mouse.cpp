//
// C++ Implementation: mouse
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "mouse.h"
#include <qstringlist.h>

#ifdef __LINUX__
#include "mouselinux.h"
#endif

QPtrList<Mouse> Mouse::m_sqInstanceList;

Mouse::Mouse() : Rotary()
{
    m_sqInstanceList.append(this);
}

Mouse::~Mouse()
{
    m_sqInstanceList.remove(this);
}

void Mouse::destroyAll()
{
    for (Mouse *p = m_sqInstanceList.first(); p; p = m_sqInstanceList.first())
        delete p;
}


QStringList Mouse::getDeviceList()
{
#ifdef __LINUX__
    return MouseLinux::getDeviceList();
#endif
}
