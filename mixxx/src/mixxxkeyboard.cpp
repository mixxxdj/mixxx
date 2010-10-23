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
#include <QList>
#include <QtDebug>
#include <QKeyEvent>
#include <QEvent>

MixxxKeyboard::MixxxKeyboard(ConfigObject<ConfigValueKbd> * pKbdConfigObject, QObject * parent, const char * name) : QObject(parent)
{
    m_pKbdConfigObject = pKbdConfigObject;
    setObjectName(name);
}

MixxxKeyboard::~MixxxKeyboard()
{
}

bool MixxxKeyboard::eventFilter(QObject *, QEvent * e)
{
    if (e->type()==QEvent::KeyPress)
    {
        QKeyEvent * ke = (QKeyEvent *)e;

        //qDebug() << "press";
        bool autoRepeat = ke->isAutoRepeat();

        if (kbdPress(getKeySeq(ke), false, autoRepeat)) {
            // Add key to active key list
            if (!autoRepeat)
                m_qActiveKeyList.append(ke->key());

            return true;
        }
    }
    else if (e->type()==QEvent::KeyRelease)
    {
        QKeyEvent * ke = (QKeyEvent *)e;

        // Run through list of active keys to see if the released key is active
        int key = -1;
        QListIterator<int> it(m_qActiveKeyList);

        while (it.hasNext())
        {
            key = it.next();
            if (key == ke->key())
            {
                //qDebug() << "release";

                bool autoRepeat = ke->isAutoRepeat();
                if (kbdPress(getKeySeq(ke), true, autoRepeat)) {
                    if (!autoRepeat) {
                        //qDebug() << "release else";
                        m_qActiveKeyList.removeOne(key);
                    }
                    return true;
                }
                return false;
            }
        }
    }

    return false;
}

bool MixxxKeyboard::kbdPress(QKeySequence k, bool release, bool autoRepeat)
{
    bool react = false;

    if (!k.isEmpty())
    {
        // Check if a shortcut is defined
        ConfigKey * pConfigKey = m_pKbdConfigObject->get(ConfigValueKbd(k));

        react = pConfigKey != NULL;

        if (pConfigKey && !autoRepeat)
        {
            if (release) {
                //qDebug() << "Sending MIDI NOTE_OFF";
                ControlObject::getControl(*pConfigKey)->queueFromMidi(NOTE_OFF, 1);
            }
            else
            {
                //qDebug() << "Sending MIDI NOTE_ON";
                ControlObject::getControl(*pConfigKey)->queueFromMidi(NOTE_ON, 1);
            }
        }
    }
    return react;
}

QKeySequence MixxxKeyboard::getKeySeq(QKeyEvent * e)
{
    //XXX: If you want Mixxx to handle multiple modifiers,
    //     eg. Ctrl+Alt+G, then you'll need to change the
    //     code below a bit.
    QKeySequence s;
    int modifier = (int)e->modifiers() & Qt::ShiftModifier;
    if ((e->modifiers() & Qt::ShiftModifier) > 0)
        s = QKeySequence("Shift+" + e->text());
    else if ((e->modifiers() & Qt::ControlModifier) > 0)
        s = QKeySequence("Ctrl+" + e->text());
    else if ((e->modifiers() & Qt::AltModifier) > 0)
        s = QKeySequence("Alt+" + e->text());
    else
        s = QKeySequence(e->key());

    //qDebug() << "keyboard press: " << s;
    return s;

}
