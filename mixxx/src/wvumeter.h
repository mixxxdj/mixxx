/***************************************************************************
                          wvumeter.h  -  description
                             -------------------
    begin                : Fri Jul 22 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#ifndef WVUMETER_H
#define WVUMETER_H

#include "wwidget.h"
#include <qpixmap.h>
#include <qstring.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WVuMeter : public WWidget  {
   Q_OBJECT
public: 
    WVuMeter(QWidget *parent=0, const char *name=0);
    ~WVuMeter();
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
    QPixmap *m_pPixmapBack, *m_pPixmapVu, *m_pPixmapBuffer;
    /** True if it's a horizontal vu meter */
    bool m_bHorizontal;
};

#endif
