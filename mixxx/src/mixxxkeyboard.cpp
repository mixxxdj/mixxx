/***************************************************************************
                          mixxxkeyboard.cpp  -  description
                             -------------------
    begin                : Wed Dec 2 2003
    copyright            : (C) 2003 by Tue Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mixxxkeyboard.h"
#include "controlobject.h"

MixxxKeyboard::MixxxKeyboard(ControlObject *control, QObject *parent, const char *name) : QObject(parent, name)
{
    m_pControl = control;
}

MixxxKeyboard::~MixxxKeyboard()
{
}

bool MixxxKeyboard::eventFilter(QObject *, QEvent *e)
{
    if (e->type()==QEvent::KeyPress)
    {
        QKeyEvent *ke = (QKeyEvent *)e;


        //qDebug("press");

        if (!m_pControl->kbdPress(QKeySequence(ke->key()), false))
            ke->ignore();
        else
        {
            // Add key to active key list
            m_qActiveKeyList.append(ke->key());

            return true;
        }
    }
    else if (e->type()==QEvent::KeyRelease)
    {
        QKeyEvent *ke = (QKeyEvent *)e;

        // Run through list of active keys to see if the released key is active
        QValueList<int>::iterator it = m_qActiveKeyList.begin();
        while (it!=m_qActiveKeyList.end())
        {
            if ((*it) == ke->key())
            {
                //qDebug("release");

                if (!ke->isAutoRepeat())
                {
                    if (!m_pControl->kbdPress(QKeySequence(ke->key()), true))
                        ke->ignore();
                    else
                    {
                        m_qActiveKeyList.remove(it);
                        return true;
                    }
                }
                return false;
            }
            ++it;
        }
    }

    return false;
}
