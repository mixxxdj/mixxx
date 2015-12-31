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
#include "widget/wpixmapstore.h"
#include "skin/skincontext.h"

class WDisplay : public WWidget {
   Q_OBJECT
  public:
    WDisplay(QWidget *parent=NULL);
    virtual ~WDisplay();

    void setup(QDomNode node, const SkinContext& context);

    void onConnectedControlChanged(double dParameter, double dValue);

  protected:
    void paintEvent(QPaintEvent*);

    int numPixmaps() const {
        return m_pixmaps.size();
    }

  private:
    void setPixmap(QVector<PaintablePointer>* pPixmaps, int iPos,
                   const QString& filename, Paintable::DrawMode mode);

    void setPixmapBackground(PixmapSource source, Paintable::DrawMode mode);

    void setPositions(int iNoPos);

    int getPixmapForParameter(double dParameter) const;

    int m_iCurrentPixmap;

    // Free existing pixmaps.
    void resetPositions();

    // Associated background pixmap
    PaintablePointer m_pPixmapBack;

    // List of associated pixmaps.
    QVector<PaintablePointer> m_pixmaps;

    // Whether disabled pixmaps are loaded.
    bool m_bDisabledLoaded;

    // List of disabled pixmaps.
    QVector<PaintablePointer> m_disabledPixmaps;
};

#endif
