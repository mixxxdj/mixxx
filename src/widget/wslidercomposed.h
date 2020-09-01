/***************************************************************************
                          wslidercomposed.h  -  description
                             -------------------
    begin                : Tue Jun 25 2002
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

#ifndef WSLIDERCOMPOSED_H
#define WSLIDERCOMPOSED_H

#include <QString>
#include <QWidget>
#include <QDomNode>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>

#include "skin/skincontext.h"
#include "util/widgetrendertimer.h"
#include "widget/slidereventhandler.h"
#include "widget/wwidget.h"
#include "widget/wpixmapstore.h"

/**
  * A widget for a slider composed of a background pixmap and a handle.
  *
  *@author Tue & Ken Haste Andersen
  */

class WSliderComposed : public WWidget  {
    Q_OBJECT
  public:
    explicit WSliderComposed(QWidget* parent = nullptr);
    ~WSliderComposed() override;

    void setup(const QDomNode& node, const SkinContext& context);
    void setSliderPixmap(
            PixmapSource sourceSlider,
            Paintable::DrawMode drawMode,
            double scaleFactor);
    void setHandlePixmap(
            bool bHorizontal,
            PixmapSource sourceHandle,
            Paintable::DrawMode mode,
            double scaleFactor);
    inline bool isHorizontal() const { return m_bHorizontal; };
    void inputActivity();

  public slots:
    void onConnectedControlChanged(double dParameter, double dValue) override;
    void fillDebugTooltip(QStringList* debug) override;

  protected:
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void drawBar(QPainter* pPainter);
    void wheelEvent(QWheelEvent* e) override;
    void resizeEvent(QResizeEvent* pEvent) override;

  private:
    double calculateHandleLength();
    void unsetPixmaps();

    // True if right mouse button is pressed.
    bool m_bRightButtonPressed;
    // Length of handle in pixels
    double m_dHandleLength;
    // Length of the slider in pixels.
    double m_dSliderLength;
    // True if it's a horizontal slider
    bool m_bHorizontal;
    // Properties to draw the level bar
    double m_dBarWidth;
    double m_dBarBgWidth;
    double m_dBarStart;
    double m_dBarEnd;
    double m_dBarBgStart;
    double m_dBarBgEnd;
    double m_dBarAxisPos;
    bool m_bBarUnipolar;
    QColor m_barColor;
    QColor m_barBgColor;
    Qt::PenCapStyle m_barPenCap;
    // Pointer to pixmap of the slider
    PaintablePointer m_pSlider;
    // Pointer to pixmap of the handle
    PaintablePointer m_pHandle;
    SliderEventHandler<WSliderComposed> m_handler;
    WidgetRenderTimer m_renderTimer;

    friend class SliderEventHandler<WSliderComposed>;
};

#endif
