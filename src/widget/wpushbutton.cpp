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

#include "widget/wpushbutton.h"

#include <QStylePainter>
#include <QStyleOption>
#include <QPixmap>
#include <QtDebug>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QApplication>

#include "widget/wpixmapstore.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "control/controlbehavior.h"
#include "util/debug.h"

const int PB_SHORTKLICKTIME = 200;

WPushButton::WPushButton(QWidget* pParent)
        : WWidget(pParent),
          m_bLeftClickForcePush(false),
          m_bRightClickForcePush(false),
          m_bPressed(false),
          m_leftButtonMode(ControlPushButton::PUSH),
          m_rightButtonMode(ControlPushButton::PUSH),
          m_indicatorConnected(false) {    
    setStates(0);
}

WPushButton::WPushButton(QWidget* pParent, ControlPushButton::ButtonMode leftButtonMode,
                         ControlPushButton::ButtonMode rightButtonMode)
        : WWidget(pParent),
          m_bLeftClickForcePush(false),
          m_bRightClickForcePush(false),
          m_leftButtonMode(leftButtonMode),
          m_rightButtonMode(rightButtonMode), 
          m_indicatorConnected(false) {
    setStates(0);
}

WPushButton::~WPushButton() {
}

void WPushButton::setup(QDomNode node, const SkinContext& context) {
    // Number of states
    int iNumStates = context.selectInt(node, "NumberStates");
    setStates(iNumStates);

    // Set background pixmap if available
    if (context.hasNode(node, "BackPath")) {
        setPixmapBackground(context.getSkinPath(context.selectString(node, "BackPath")));
    }

    // Load pixmaps for associated states
    QDomNode state = context.selectNode(node, "State");
    while (!state.isNull()) {
        if (state.isElement() && state.nodeName() == "State") {
            int iState = context.selectInt(state, "Number");
            if (iState >= m_iNoStates) {
                qDebug() << "WPushButton provided state info for invalid state:" << iState;
                continue;
            }

            if (context.hasNode(state, "Pressed")) {
                setPixmap(iState, true,
                          context.getSkinPath(context.selectString(state, "Pressed")));
            }
            if (context.hasNode(state, "Unpressed")) {
                setPixmap(iState, false,
                          context.getSkinPath(context.selectString(state, "Unpressed")));
            }
            m_text[iState] = context.selectString(state, "Text");
        }
        state = state.nextSibling();
    }

    m_bLeftClickForcePush = context.selectBool(node, "LeftClickIsPushButton", false);
    m_bRightClickForcePush = context.selectBool(node, "RightClickIsPushButton", false);

    QDomNode con = context.selectNode(node, "Connection");
    while (!con.isNull()) {
        // Get ConfigKey
        QString key = context.selectString(con, "ConfigKey");

        ConfigKey configKey;
        configKey.group = key.left(key.indexOf(","));
        configKey.item = key.mid(key.indexOf(",")+1);

        bool isLeftButton = false;
        bool isRightButton = false;
        if (context.hasNode(con, "ButtonState")) {
            if (context.selectString(con, "ButtonState").contains("LeftButton", Qt::CaseInsensitive)) {
                isLeftButton = true;
            } else if (context.selectString(con, "ButtonState").contains("RightButton", Qt::CaseInsensitive)) {
                isRightButton = true;
            }
        }

        ControlPushButton* p = dynamic_cast<ControlPushButton*>(
            ControlObject::getControl(configKey));

        if (p) {
            // A NULL here either means that this control is not a
            // ControlPushButton or it does not exist. This logic is
            // specific to push-buttons, so skip it either way.

            // Based on whether the control is mapped to the left or right button,
            // record the button mode.
            if (isLeftButton) {
                m_leftButtonMode = p->getButtonMode();
            } else if (isRightButton) {
                m_rightButtonMode = p->getButtonMode();
            }
        }
        con = con.nextSibling();
    }
}

void WPushButton::setStates(int iStates) {
    m_bPressed = false;
    m_iNoStates = 0;

    // Clear existing pixmaps.
    m_pressedPixmaps.resize(0);
    m_unpressedPixmaps.resize(0);
    m_text.resize(0);

    if (iStates > 0) {
        m_iNoStates = iStates;
        m_pressedPixmaps.resize(iStates);
        m_unpressedPixmaps.resize(iStates);
        m_text.resize(iStates);
    }
}

void WPushButton::setPixmap(int iState, bool bPressed, const QString &filename) {

    QVector<PaintablePointer>& pixmaps = bPressed ?
            m_pressedPixmaps : m_unpressedPixmaps;

    if (iState < 0 || iState >= pixmaps.size()) {
        return;
    }

    PaintablePointer pPixmap = WPixmapStore::getPaintable(filename);

    if (pPixmap.isNull() || pPixmap->isNull()) {
        qDebug() << "WPushButton: Error loading pixmap:" << filename;
    } else {
        pixmaps[iState] = pPixmap;

        // Set size of widget equal to pixmap size
        setFixedSize(pPixmap->size());
    }
}

void WPushButton::setPixmapBackground(const QString &filename) {
    // Load background pixmap
    m_pPixmapBack = WPixmapStore::getPaintable(filename);
    if (m_pPixmapBack.isNull() || m_pPixmapBack->isNull()) {
        qDebug() << "WPushButton: Error loading background pixmap:" << filename;
    }
}

void WPushButton::onConnectedControlValueChanged(double v) {
    if (m_iNoStates == 1) {
        m_bPressed = (v == 1.0);
    }
    update();
}

void WPushButton::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    double value = getControlParameterDisplay();
    if (m_iNoStates == 0) {
        return;
    }

    const QVector<PaintablePointer>& pixmaps = m_bPressed ?
            m_pressedPixmaps : m_unpressedPixmaps;

    int idx = static_cast<int>(value) % m_iNoStates;

    // Just in case m_iNoStates is somehow different from pixmaps.size().
    if (idx < 0 || idx >= pixmaps.size()) {
        return;
    }

    if (m_pPixmapBack) {
        m_pPixmapBack->draw(0, 0, &p);
    }

    PaintablePointer pPixmap = pixmaps[idx];
    if (!pPixmap.isNull() && !pPixmap->isNull()) {
        pPixmap->draw(0, 0, &p);
    }

    QString text = m_text[idx];
    if (!text.isEmpty()) {
        p.drawText(rect(), Qt::AlignCenter, text);
    }
}

void WPushButton::mousePressEvent(QMouseEvent * e) {
    const bool leftClick = e->button() == Qt::LeftButton;
    const bool rightClick = e->button() == Qt::RightButton;
    const bool leftPowerWindowStyle = m_leftButtonMode == ControlPushButton::POWERWINDOW;
    const bool leftLongPressLatchingStyle = m_leftButtonMode == ControlPushButton::LONGPRESSLATCHING;

    if (leftPowerWindowStyle && m_iNoStates == 2) {
        if (leftClick) {
            if (getControlParameterLeft() == 0.0) {
                m_clickTimer.setSingleShot(true);
                m_clickTimer.start(ControlPushButtonBehavior::kPowerWindowTimeMillis);
            }
            m_bPressed = true;
            setControlParameterLeftDown(1.0);
            update();
        }
        // discharge right clicks here, because is used for latching in POWERWINDOW mode
        return;
    }

    if (rightClick) {
        // This is the secondary button function, it does not change m_fValue
        // due the leak of visual feedback we do not allow a toggle function
        if (m_bRightClickForcePush) {
            m_bPressed = true;
            setControlParameterRightDown(1.0);
            update();
        } else if (m_iNoStates == 1) {
            // This is a Pushbutton
            m_bPressed = true;
            setControlParameterRightDown(1.0);
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
        if (m_bLeftClickForcePush || m_iNoStates == 1) {
            // This is either forced to behave like a push button on left-click
            // or this is a push button.
            emitValue = 1.0;
        } else {
            // Toggle thru the states
            emitValue = static_cast<int>(getControlParameterLeft() + 1.0) % m_iNoStates;
            if (leftLongPressLatchingStyle) {
                m_clickTimer.setSingleShot(true);
                m_clickTimer.start(ControlPushButtonBehavior::kLongPressLatchingTimeMillis);
            }
        }
        m_bPressed = true;
        setControlParameterLeftDown(emitValue);
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
    const bool leftLongPressLatchingStyle = m_leftButtonMode == ControlPushButton::LONGPRESSLATCHING;

    if (leftPowerWindowStyle && m_iNoStates == 2) {
        if (leftClick) {
            const bool rightButtonDown = QApplication::mouseButtons() & Qt::RightButton;
            if (m_bPressed && !m_clickTimer.isActive() && !rightButtonDown) {
                // Release button after timer, but not if right button is clicked
                setControlParameterLeftUp(0.0);
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
            setControlParameterRightUp(0.0);
            update();
        } else if (m_iNoStates == 1) {
            m_bPressed = false;
            setControlParameterRightUp(0.0);
            update();
        }
        return;
    }

    if (leftClick) {
        double emitValue = getControlParameterLeft();
        if (m_bLeftClickForcePush) {
            // This may a klickButton with different functions on each mouse button
            // m_fValue is changed by a separate feedback connection
            emitValue = 0.0;
        } else if (m_iNoStates == 1) {
            // This is a Pushbutton
            emitValue = 0.0;
        } else {
            if (leftLongPressLatchingStyle && m_clickTimer.isActive() && emitValue >= 1.0) {
                // revert toggle if button is released too early
                emitValue = static_cast<int>(emitValue - 1.0) % m_iNoStates;
            } else {
                // Nothing special happens when releasing a normal toggle button
            }
        }
        m_bPressed = false;
        setControlParameterLeftUp(emitValue);
        update();
    }
}

void WPushButton::fillDebugTooltip(QStringList* debug) {
    WWidget::fillDebugTooltip(debug);
    *debug << QString("NumberStates: %1").arg(m_iNoStates)
           << QString("LeftCurrentState: %1").arg(
               static_cast<int>(getControlParameterLeft()) %
               (m_iNoStates > 0 ? m_iNoStates : 1))
           << QString("Pressed: %1").arg(toDebugString(m_bPressed))
           << QString("LeftButtonMode: %1")
            .arg(ControlPushButton::buttonModeToString(m_leftButtonMode))
           << QString("RightButtonMode: %1")
            .arg(ControlPushButton::buttonModeToString(m_rightButtonMode))
           << QString("LeftClickForcePush: %1")
            .arg(toDebugString(m_bLeftClickForcePush))
           << QString("RightClickForcePush: %1")
            .arg(toDebugString(m_bRightClickForcePush));
}
