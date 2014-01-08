/***************************************************************************
                          wpushbutton.h  -  description
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

#ifndef WPUSHBUTTON_H
#define WPUSHBUTTON_H

#include <QPaintEvent>
#include <QPixmap>
#include <QString>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QTimer>
#include <QVector>

#include "widget/wwidget.h"
#include "widget/wpixmapstore.h"
#include "controlpushbutton.h"
#include "skin/skincontext.h"

class WPushButton : public WWidget {
    Q_OBJECT
  public:
    WPushButton(QWidget* pParent=NULL);
    // Used by WPushButtonTest.
    WPushButton(QWidget* pParent, ControlPushButton::ButtonMode leftButtonMode,
                ControlPushButton::ButtonMode rightButtonMode);
    virtual ~WPushButton();

    Q_PROPERTY(bool pressed READ isPressed);

    bool isPressed() const {
        return m_bPressed;
    }

    void setup(QDomNode node, const SkinContext& context);

    // Sets the number of states associated with this button, and removes
    // associated pixmaps.
    void setStates(int iStatesW);

  public slots:
    void slotConnectedValueChanged(double);

  protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void focusOutEvent(QFocusEvent* e);

  private:
    // Associates a pixmap of a given state of the button with the widget
    void setPixmap(int iState, bool bPressed, const QString &filename);

    // Associates a background pixmap with the widget. This is only needed if
    // the button pixmaps contains alpha channel values.
    void setPixmapBackground(const QString &filename);

    bool m_bLeftClickForcePush;
    bool m_bRightClickForcePush;

    // True, if the button is currently pressed
    bool m_bPressed;

    // Array of associated pixmaps
    int m_iNoStates;
    QVector<QString> m_text;
    QVector<PaintablePointer> m_pressedPixmaps;
    QVector<PaintablePointer> m_unpressedPixmaps;

    // Associated background pixmap
    PaintablePointer m_pPixmapBack;

    // short click toggle button long click push button
    ControlPushButton::ButtonMode m_leftButtonMode;
    ControlPushButton::ButtonMode m_rightButtonMode;
    QTimer m_clickTimer;
};

#endif
