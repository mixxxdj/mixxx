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

#include "wwidget.h"
#include <qpixmap.h>
#include <qstring.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WKnob : public WWidget  {
   Q_OBJECT
public: 
    WKnob(QWidget *parent=0, const char *name=0);
    ~WKnob();
    void setPositions(int iNoPos);
    void setPixmap(int iPos, const QString &filename);
    
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
    /** Values used when pressing mouse */
    float m_fOldValue, m_fStartValue;
    
};

#endif
