/***************************************************************************
                          wknob.h  -  description
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

#ifndef WKNOB_H
#define WKNOB_H

#include "wabstractcontrol.h"
#include <qpixmap.h>
#include <qstring.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>

/**
  *@author Tue & Ken Haste Andersen
  */

class WKnob : public WAbstractControl  {
   Q_OBJECT
public:
    WKnob(QWidget *parent=0);
    ~WKnob();
    void setup(QDomNode node);
    void setPositions(int iNoPos, bool bIncludingDisabled=false);
    void setPixmap(int iPos, const QString &filename);
    /** Associates a background pixmap with the widget. This is only needed if the knob
      * pixmaps contains alpha channel values. */
    void setPixmapBackground(const QString &filename);
    void wheelEvent(QWheelEvent *e);
private:
    /** Set position number to zero and deallocate pixmaps */
    void resetPositions();
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);

    /** Current position */
    int m_iPos;
    /** Number of positions associated with this knob */
    int m_iNoPos;
    /** Array of associated pixmaps */
    QPixmap **m_pPixmaps;
    /** Associated background pixmap */
    QPixmap *m_pPixmapBack;
    /** Starting point when left mouse button is pressed */
    QPoint m_startPos;
    /** True if disabled pixmaps is loaded */
    bool m_bDisabledLoaded;
};

#endif
