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

#include <QPixmap>
#include <QString>
#include <QPaintEvent>
#include <QWidget>
#include <QDomNode>

#include "widget/wwidget.h"
#include "widget/wpixmapstore.h"
#include "skin/skincontext.h"
#include "util/performancetimer.h"

class WVuMeter : public WWidget  {
   Q_OBJECT
  public:
    WVuMeter(QWidget *parent=0);
    virtual ~WVuMeter();

    void setup(QDomNode node, const SkinContext& context);
    void setPixmapBackground(PixmapSource source);
    void setPixmaps(PixmapSource source,
                    bool bHorizontal=false);
    void onConnectedControlChanged(double dParameter, double dValue);

  protected slots:
    void updateState(double msecsElapsed);
    void maybeUpdate();

  private:
    /** Set position number to zero and deallocate pixmaps */
    void resetPositions();
    void paintEvent(QPaintEvent *);
    void setPeak(int pos, double parameter);

    // Current position in the pixmap.
    int m_iPos;
    double m_dParameter;
    // Number of positions in the pixmap.
    int m_iNoPos;
    // Current position in the widget.
    int m_iWidgetPos;


    /** Associated pixmaps */
    PaintablePointer m_pPixmapBack;
    PaintablePointer m_pPixmapVu;
    /** True if it's a horizontal vu meter */
    bool m_bHorizontal;

    int m_iPeakHoldSize;
    int m_iPeakFallStep;
    int m_iPeakHoldTime;
    int m_iPeakFallTime;
    int m_iPeakPos;
    double m_dPeakParameter;
    int m_iPeakHoldCountdown;
    int m_iLastPos;
    int m_iLastPeakPos;

    PerformanceTimer m_timer;
};

#endif
