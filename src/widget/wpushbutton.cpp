/***************************************************************************
                          wpushbutton.cpp  -  description
                             -------------------
    begin                : Fri Jun 21 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "wpushbutton.h"
#include "wpixmapstore.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "control/controlbehavior.h"
//Added by qt3to4:
#include <QPixmap>
#include <QtDebug>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QApplication>

const int PB_SHORTKLICKTIME = 200;

WPushButton::WPushButton(QWidget * parent) :
        WWidget(parent),
        m_pPixmaps(NULL),
        m_pPixmapBack(NULL),
        m_leftButtonMode(ControlPushButton::PUSH),
        m_rightButtonMode(ControlPushButton::PUSH) {
    setStates(0);
    //setBackgroundMode(Qt::NoBackground); //obsolete? removal doesn't seem to change anything on the GUI --kousu 2009/03
}

WPushButton::~WPushButton() {
    for (int i = 0; i < 2*m_iNoStates; i++) {
        WPixmapStore::deletePixmap(m_pPixmaps[i]);
    }

    delete [] m_pPixmaps;

    WPixmapStore::deletePixmap(m_pPixmapBack);
}

void WPushButton::setup(QDomNode node) {
    // Number of states
    int iNumStates = selectNodeInt(node, "NumberStates");
    setStates(iNumStates);

    // Set background pixmap if available
    if (!selectNode(node, "BackPath").isNull()) {
        setPixmapBackground(getPath(selectNodeQString(node, "BackPath")));
    }

    // Load pixmaps for associated states
    QDomNode state = selectNode(node, "State");
    while (!state.isNull()) {
        if (state.isElement() && state.nodeName() == "State") {
            setPixmap(selectNodeInt(state, "Number"), true, getPath(selectNodeQString(state, "Pressed")));
            setPixmap(selectNodeInt(state, "Number"), false, getPath(selectNodeQString(state, "Unpressed")));
        }
        state = state.nextSibling();
    }

    m_bLeftClickForcePush = selectNodeQString(node, "LeftClickIsPushButton")
            .contains("true", Qt::CaseInsensitive);

    m_bRightClickForcePush = selectNodeQString(node, "RightClickIsPushButton")
            .contains("true", Qt::CaseInsensitive);


    QDomNode con = selectNode(node, "Connection");
    while (!con.isNull()) {
        // Get ConfigKey
        QString key = selectNodeQString(con, "ConfigKey");

        ConfigKey configKey;
        configKey.group = key.left(key.indexOf(","));
        configKey.item = key.mid(key.indexOf(",")+1);

        ControlPushButton* p = dynamic_cast<ControlPushButton*>(
            ControlObject::getControl(configKey));

        if (p == NULL) {
            // A NULL here either means that this control is not a
            // ControlPushButton or it does not exist. This logic is
            // specific to push-buttons, so skip it either way.
            con = con.nextSibling();
            continue;
        }

        bool isLeftButton = false;
        bool isRightButton = false;
        if (!selectNode(con, "ButtonState").isNull()) {
            if (selectNodeQString(con, "ButtonState").contains("LeftButton", Qt::CaseInsensitive)) {
                isLeftButton = true;
            } else if (selectNodeQString(con, "ButtonState").contains("RightButton", Qt::CaseInsensitive)) {
                isRightButton = true;
            }
        }

        // Based on whether the control is mapped to the left or right button,
        // record the button mode.
        if (isLeftButton) {
            m_leftButtonMode = p->getButtonMode();
        } else if (isRightButton) {
            m_rightButtonMode = p->getButtonMode();
        }
        con = con.nextSibling();
    }
}

void WPushButton::setStates(int iStates) {
    m_iNoStates = iStates;
    m_fValue = 0.;
    m_bPressed = false;

    // If pixmap array is already allocated, delete it
    delete [] m_pPixmaps;
    m_pPixmaps = NULL;

    if (iStates > 0) {
        m_pPixmaps = new QPixmap*[2 * m_iNoStates];
        for (int i = 0; i < (2 * m_iNoStates); ++i) {
            m_pPixmaps[i] = NULL;
        }
    }
}

void WPushButton::setPixmap(int iState, bool bPressed, const QString &filename) {
    int pixIdx = (iState * 2) + (bPressed ? 1 : 0);
    if (pixIdx < 2 * m_iNoStates) {
        m_pPixmaps[pixIdx] = WPixmapStore::getPixmap(filename);
        if (!m_pPixmaps[pixIdx]) {
            qDebug() << "WPushButton: Error loading pixmap:" << filename;
        } else {
            // Set size of widget equal to pixmap size
            setFixedSize(m_pPixmaps[pixIdx]->size());
        }
    }
}

void WPushButton::setPixmapBackground(const QString &filename) {
    // Load background pixmap
    m_pPixmapBack = WPixmapStore::getPixmap(filename);
    if (!m_pPixmapBack) {
        qDebug() << "WPushButton: Error loading background pixmap:" << filename;
    }
}

void WPushButton::setValue(double v) {
    m_fValue = v;

    if (m_iNoStates==1) {
        if (m_fValue==1.) {
            m_bPressed = true;
        } else {
            m_bPressed = false;
        }
    }
    update();
}

void WPushButton::paintEvent(QPaintEvent *) {
    if (m_iNoStates>0)     {
        int idx = (((int)m_fValue % m_iNoStates) * 2) + m_bPressed;
        if (m_pPixmaps[idx]) {
            QPainter p(this);
            if(m_pPixmapBack) {
                p.drawPixmap(0, 0, *m_pPixmapBack);
            }
            p.drawPixmap(0, 0, *m_pPixmaps[idx]);
        }
    }
}

void WPushButton::mousePressEvent(QMouseEvent * e) {
    const bool leftClick = e->button() == Qt::LeftButton;
    const bool rightClick = e->button() == Qt::RightButton;

    const bool leftPowerWindowStyle = m_leftButtonMode == ControlPushButton::POWERWINDOW;
    if (leftPowerWindowStyle && m_iNoStates == 2) {
        if (leftClick) {
            if (m_fValue == 0.0f) {
                m_clickTimer.setSingleShot(true);
                m_clickTimer.start(ControlPushButtonBehavior::kPowerWindowTimeMillis);
            }
            m_fValue = 1.0f;
            m_bPressed = true;
            emit(valueChangedLeftDown(1.0f));
            update();
        }
        return;
    }

    if (rightClick) {
        // This is the secondary button function, it does not change m_fValue
        // due the leak of visual feedback we do not allow a toggle function
        if (m_bRightClickForcePush) {
            m_bPressed = true;
            emit(valueChangedRightDown(1.0f));
            update();
        } else if (m_iNoStates == 1) {
            // This is a Pushbutton
            m_fValue = 1.0f;
            m_bPressed = true;
            emit(valueChangedRightDown(1.0f));
            update();
        }

        // Do not allow right-clicks to change button state other than when
        // forced to be a push button. This is how Mixxx <1.8.0 worked so
        // keep it that way. For a multi-state button, really only one click
        // type (left/right) should be able to change the state. One problem
        // with this is that you can get the button out of sync with its
        // underlying control. For example the PFL buttons on Jus's skins
        // could get out of sync with the button state. rryan 9/2010
        return;
    }

    if (leftClick) {
        double emitValue;
        if (m_bLeftClickForcePush) {
            // This may a button with different functions on each mouse button
            // m_fValue is changed by a separate feedback connection
            emitValue = 1.0f;
        } else if (m_iNoStates == 1) {
            // This is a Pushbutton
            m_fValue = emitValue = 1.0f;
        } else {
            // Toggle thru the states
            m_fValue = emitValue = (int)(m_fValue+1.)%m_iNoStates;
        }
        m_bPressed = true;
        emit(valueChangedLeftDown(emitValue));
        update();
    }
}

void WPushButton::focusOutEvent(QFocusEvent* e) {
    Q_UNUSED(e);
    m_bPressed = false;
    update();
}

void WPushButton::mouseReleaseEvent(QMouseEvent * e) {
    const bool leftClick = e->button() == Qt::LeftButton;
    const bool rightClick = e->button() == Qt::RightButton;
    const bool leftPowerWindowStyle = m_leftButtonMode == ControlPushButton::POWERWINDOW;

    if (leftPowerWindowStyle && m_iNoStates == 2) {
        if (leftClick) {
            const bool rightButtonDown = QApplication::mouseButtons() & Qt::RightButton;
            if (m_bPressed && !m_clickTimer.isActive() && !rightButtonDown) {
                // Release Button after Timer, but not if right button is clicked
                m_fValue = 0.0f;
                emit(valueChangedLeftUp(0.0f));
            }
            m_bPressed = false;
        } else if (rightClick) {
            m_bPressed = false;
        }
        update();
        return;
    }

    if (rightClick) {
        // This is the secondary clickButton function, it does not change
        // m_fValue due the leak of visual feedback we do not allow a toggle
        // function
        if (m_bRightClickForcePush) {
            m_bPressed = false;
            emit(valueChangedRightDown(0.0f));
            update();
        } else if (m_iNoStates == 1) {
            m_bPressed = false;
            emit(valueChangedRightDown(0.0f));
            update();
        }
        return;
    }

    if (leftClick) {
        double emitValue = m_fValue;
        if (m_bLeftClickForcePush) {
            // This may a klickButton with different functions on each mouse button
            // m_fValue is changed by a separate feedback connection
            emitValue = 0.0f;
        } else if (m_iNoStates == 1) {
            // This is a Pushbutton
            m_fValue = emitValue = 0.0f;
        } else {
            // Nothing special happens when releasing a toggle button
        }
        m_bPressed = false;
        emit(valueChangedLeftDown(emitValue));
        update();
    }
}
