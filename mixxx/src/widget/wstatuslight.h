/***************************************************************************
                          wstatuslight.h  -  A general purpose status light
                                        for indicating boolean events
                             -------------------
    begin                : Fri Jul 22 2007
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
                           (C) 2007 by John Sully (derived from WVumeter)
    email                : jsully@scs.ryerson.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WSTATUSLIGHT_H
#define WSTATUSLIGHT_H

#include "wwidget.h"
#include <qpixmap.h>
#include <qstring.h>
//Added by qt3to4:
#include <QPaintEvent>

/**
  *@author John Sully
  */

class WStatusLight : public WWidget  {
   Q_OBJECT
public: 
    WStatusLight(QWidget *parent=0);
    ~WStatusLight();
    void setup(QDomNode node);
    void setPixmaps(const QString &backFilename, const QString &vuFilename, bool bHorizontal=false);
    
private:
    /** Set position number to zero and deallocate pixmaps */
    void resetPositions();
    void paintEvent(QPaintEvent *);

    /** Current position */
    int m_iPos;
    /** Number of positions associated with this knob */
    int m_iNoPos;
    /** Associated pixmaps */
    QPixmap *m_pPixmapBack, *m_pPixmapSL;
    /** True if it's a horizontal vu meter */
    bool m_bHorizontal;
};

#endif
