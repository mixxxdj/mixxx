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

#include <QPaintEvent>
#include <QWidget>
#include <QString>
#include <QDomNode>
#include <QPixmap>
#include <QVector>

#include "widget/wwidget.h"
#include "widget/wpixmapstore.h"
#include "skin/skincontext.h"

class WStatusLight : public WWidget  {
   Q_OBJECT
  public:
    WStatusLight(QWidget *parent=0);
    virtual ~WStatusLight();

    void setup(QDomNode node, const SkinContext& context);

  public slots:
    void onConnectedControlChanged(double dParameter, double dValue);

  protected:
    void paintEvent(QPaintEvent *);

  private:
    void setPixmap(int iState, PixmapSource source, Paintable::DrawMode mode);
    void setNoPos(int iNoPos);

    // Current position
    int m_iPos;

    PaintablePointer m_pPixmapBackground;

    // Associated pixmaps
    QVector<PaintablePointer> m_pixmaps;
};

#endif
