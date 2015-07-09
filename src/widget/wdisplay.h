/***************************************************************************
                          wdisplay.h  -  description
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

#ifndef WDISPLAY_H
#define WDISPLAY_H

#include <QPixmap>
#include <QPaintEvent>
#include <QString>

#include "widget/wwidget.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class WDisplay : public WWidget  {
   Q_OBJECT
public:
    WDisplay(QWidget *parent=0);
    ~WDisplay();
    void setup(QDomNode node);
    void setPositions(int iNoPos);
    void setPixmap(int iPos, const QString &filename);

private:
    /** Set position number to zero and deallocate pixmaps */
    void resetPositions();
    void paintEvent(QPaintEvent *);

    /** Current position */
    int m_iPos;
    /** Number of positions associated with this knob */
    int m_iNoPos;
    /** Array of associated pixmaps */
    QPixmap **m_pPixmaps;
    };

#endif
