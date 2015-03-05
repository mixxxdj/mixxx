/***************************************************************************
                          mixxxkeyboard.h  -  description
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

#ifndef MIXXXKEYBOARD_H
#define MIXXXKEYBOARD_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QMultiHash>

#include "configobject.h"

class ControlObject;

// This class provides handling of keyboard events.
class MixxxKeyboard : public QObject {
    Q_OBJECT
  public:
    MixxxKeyboard(ConfigObject<ConfigValueKbd> *pKbdConfigObject,
                  QObject *parent=NULL, const char* name=NULL);
    virtual ~MixxxKeyboard();

    bool eventFilter(QObject *obj, QEvent *e);

    // Set the keyboard config object. MixxxKeyboard does NOT take ownership of
    // pKbdConfigObject.
    void setKeyboardConfig(ConfigObject<ConfigValueKbd> *pKbdConfigObject);
    ConfigObject<ConfigValueKbd>* getKeyboardConfig();

  private:
    struct KeyDownInformation {
        KeyDownInformation(int keyId, int modifiers, ControlObject* pControl)
                : keyId(keyId),
                  modifiers(modifiers),
                  pControl(pControl) {
        }

        int keyId;
        int modifiers;
        ControlObject* pControl;
    };

    // Returns a valid QString with modifier keys from a QKeyEvent
    QKeySequence getKeySeq(QKeyEvent *e);
    // List containing keys which is currently pressed
    QList<KeyDownInformation> m_qActiveKeyList;
    // Pointer to keyboard config object
    ConfigObject<ConfigValueKbd> *m_pKbdConfigObject;
    // Multi-hash of key sequence to
    QMultiHash<ConfigValueKbd, ConfigKey> m_keySequenceToControlHash;
};

#endif
