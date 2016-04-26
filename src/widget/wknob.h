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

#include <QPixmap>
#include <QString>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include "widget/wdisplay.h"
#include "widget/knobeventhandler.h"

class WKnob : public WDisplay {
   Q_OBJECT
  public:
    explicit WKnob(QWidget* pParent=nullptr);

  protected:
    void wheelEvent(QWheelEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

  private:
    KnobEventHandler<WKnob> m_handler;
    friend class KnobEventHandler<WKnob>;
};

#endif
