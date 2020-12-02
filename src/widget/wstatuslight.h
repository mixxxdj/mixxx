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

#include <QByteArrayData>
#include <QDomNode>
#include <QPaintEvent>
#include <QPixmap>
#include <QString>
#include <QVector>
#include <QWidget>

#include "skin/pixmapsource.h"
#include "skin/skincontext.h"
#include "widget/paintable.h"
#include "widget/wpixmapstore.h"
#include "widget/wwidget.h"

class QObject;
class QPaintEvent;
class QWidget;

class WStatusLight : public WWidget  {
   Q_OBJECT
  public:
    explicit WStatusLight(QWidget *parent=nullptr);

    void setup(const QDomNode& node, const SkinContext& context);

  public slots:
    void onConnectedControlChanged(double dParameter, double dValue) override;

  protected:
    void paintEvent(QPaintEvent * /*unused*/) override;

  private:
    void setPixmap(int iState,
            const PixmapSource& source,
            Paintable::DrawMode mode,
            double scaleFactor);
    void setNoPos(int iNoPos);

    // Current position
    int m_iPos;

    PaintablePointer m_pPixmapBackground;

    // Associated pixmaps
    QVector<PaintablePointer> m_pixmaps;
};

#endif
