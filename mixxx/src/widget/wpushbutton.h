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

#include "wwidget.h"
#include <qpainter.h>
#include <qpixmap.h>
#include <qstring.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>

/**
  *@author Tue & Ken Haste Andersen
  */

class WPushButton : public WWidget
{
    Q_OBJECT
public:
    WPushButton(QWidget *parent=0);
    ~WPushButton();
    void setup(QDomNode node);
    /** Sets the number of states associated with this button, and removes associated
      * pixmaps. */
    void setStates(int iStatesW);
    /** Associates a pixmap of a given state of the button with the widget */
    void setPixmap(int iState, bool bPressed, const QString &filename);
    /** Associates a background pixmap with the widget. This is only needed if the button
      * pixmaps contains alpha channel values. */
    void setPixmapBackground(const QString &filename);
    /** Paints the widget */
    void paintEvent(QPaintEvent *);
    /** Mouse pressed */
    void mousePressEvent(QMouseEvent *e);
    /** Mouse released */
    void mouseReleaseEvent(QMouseEvent *e);
public slots:
    void setValue(double);
protected:
    /** True, if the button is currently pressed */
    bool m_bPressed;
private:
    bool m_bLeftClickForcePush, m_bRightClickForcePush;
    /** Number of states associated with this button */
    int m_iNoStates;
    /** Array of associated pixmaps */
    QPixmap **m_pPixmaps;
    /** Associated background pixmap */
    QPixmap *m_pPixmapBack;
};

#endif
