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
   // TODO(XXX) ugly workaround to get no leak
   delete m_pKbdConfigObject;
}

bool MixxxKeyboard::eventFilter(QObject *, QEvent * e) {
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent * ke = (QKeyEvent *)e;

#ifdef __OSX__
        // On Mac OSX the nativeScanCode is empty http://doc.qt.nokia.com/4.7/qkeyevent.html#nativeScanCode
        // We may loose the release event if a the shift key is pressed later
        // and there is character shift like "1" -> "!"
        int keyId = ke->key();
#else
        int keyId = ke->nativeScanCode();
#endif
        //qDebug() << "key event =" << ke->key() << "KeyId =" << keyId;

        // Run through list of active keys to see if the pressed key is already active
        // Just for returning true if we are consuming this key event
        QListIterator<QPair<int, ConfigKey *> > it(m_qActiveKeyList);
        while (it.hasNext()) {
            if (it.next().first == keyId) {
                return true;
            }
        }

        QString keystring = getKeySeq(ke);
        if (!keystring.isEmpty())
        {
            // Check if a shortcut is defined
            ConfigKey * pConfigKey = m_pKbdConfigObject->get(ConfigValueKbd(keystring));

            if (pConfigKey)
            {
                ControlObject::getControl(*pConfigKey)->queueFromMidi(MIDI_NOTE_ON, 1);
                // Add key to active key list
                m_qActiveKeyList.append(QPair<int, ConfigKey *>(keyId,pConfigKey));
                return true;
            }
        }
    } else if (e->type()==QEvent::KeyRelease) {
        QKeyEvent * ke = (QKeyEvent *)e;

#ifdef __OSX__
        // On Mac OSX the nativeScanCode is empty
        int keyId = ke->key();
#else
        int keyId = ke->nativeScanCode();
#endif
        bool autoRepeat = ke->isAutoRepeat();

        //qDebug() << "key event =" << ke->key() << "AutoRepeat =" << autoRepeat << "KeyId =" << keyId;

        // Run through list of active keys to see if the released key is active
        for (int i = m_qActiveKeyList.size() - 1; i >= 0; i--) {
            if (m_qActiveKeyList[i].first == keyId) {
                if(!autoRepeat) {
                    ControlObject::getControl(*(m_qActiveKeyList[i].second))->queueFromMidi(MIDI_NOTE_OFF, 0);
                    m_qActiveKeyList.removeAt(i);
                }
                return true;
            }
        }
    } else {
        if (e->type() == QEvent::KeyboardLayoutChange) {
            // This event is not fired on ubunty natty, why?
			// TODO(XXX): find a way to support KeyboardLayoutChange Bug #997811 
            qDebug() << "QEvent::KeyboardLayoutChange";
        }
    }
    return false;
}

QString MixxxKeyboard::getKeySeq(QKeyEvent * e) {
    QString modseq;
	QString keyseq;

	if (e->modifiers() & Qt::ShiftModifier)
               modseq += "Shift+";

	if (e->modifiers() & Qt::ControlModifier)
		modseq += "Ctrl+";

	if (e->modifiers() & Qt::AltModifier)
		modseq += "Alt+";

	if (e->modifiers() & Qt::MetaModifier)
		modseq += "Meta+";

	keyseq = e->text();

	if (!keyseq.isEmpty()) { // avoid returning e.g. "Meta+"
        keyseq = modseq + keyseq;
    }
	qDebug() << "keyboard press: " << keyseq;
	return keyseq;
}

void MixxxKeyboard::setKeyboardConfig(ConfigObject<ConfigValueKbd> * pKbdConfigObject) {
    m_pKbdConfigObject = pKbdConfigObject;
}

ConfigObject<ConfigValueKbd>* MixxxKeyboard::getKeyboardConfig() {
    return m_pKbdConfigObject;
}
