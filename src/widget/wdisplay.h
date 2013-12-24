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

#include <QVector>
#include <QPixmap>
#include <QPaintEvent>
#include <QString>

#include "widget/wwidget.h"

class WDisplay : public WWidget  {
   Q_OBJECT
  public:
    WDisplay(QWidget *parent=0);
    virtual ~WDisplay();

    void setup(QDomNode node);

    void setPixmap(int iPos, const QString& filename);

  private:
    void setPositions(int iNoPos);

    // Free existing pixmaps.
    void resetPositions();

    void paintEvent(QPaintEvent*);

    // List of associated pixmaps.
    QVector<QPixmap*> m_pixmaps;
};

#endif
