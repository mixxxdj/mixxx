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
    void setPixmapBackground(PixmapSource source, Paintable::DrawMode mode);
    void setPixmaps(PixmapSource source,
                    bool bHorizontal,
                    Paintable::DrawMode mode);
    void onConnectedControlChanged(double dParameter, double dValue);

  protected slots:
    void updateState(double msecsElapsed);
    void maybeUpdate();

  private:
    void paintEvent(QPaintEvent *);
    void setPeak(double parameter);

    // Current parameter and peak parameter.
    double m_dParameter;
    double m_dPeakParameter;

    // The last parameter and peak parameter values at the time of
    // rendering. Used to check whether the widget state has changed since the
    // last render in maybeUpdate.
    double m_dLastParameter;
    double m_dLastPeakParameter;

    // Length of the VU-meter pixmap along the relevant axis.
    int m_iPixmapLength;

    // Associated pixmaps
    PaintablePointer m_pPixmapBack;
    PaintablePointer m_pPixmapVu;

    // True if it's a horizontal vu meter
    bool m_bHorizontal;

    int m_iPeakHoldSize;
    int m_iPeakFallStep;
    int m_iPeakHoldTime;
    int m_iPeakFallTime;

    // The peak hold time remaining in milliseconds.
    double m_dPeakHoldCountdownMs;

    PerformanceTimer m_timer;
};

#endif
