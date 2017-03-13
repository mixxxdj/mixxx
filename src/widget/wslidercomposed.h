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

#include "widget/slidereventhandler.h"
#include "widget/wwidget.h"
#include "widget/wpixmapstore.h"
#include "skin/skincontext.h"

/**
  * A widget for a slider composed of a background pixmap and a handle.
  *
  *@author Tue & Ken Haste Andersen
  */

class WSliderComposed : public WWidget  {
    Q_OBJECT
  public:
    WSliderComposed(QWidget* parent = 0);
    virtual ~WSliderComposed();

    void setup(QDomNode node, const SkinContext& context);
    void setSliderPixmap(PixmapSource sourceSlider, Paintable::DrawMode mode);
    void setHandlePixmap(bool bHorizontal, PixmapSource sourceHandle,
                         Paintable::DrawMode mode);
    inline bool isHorizontal() const { return m_bHorizontal; };

  public slots:
    void onConnectedControlChanged(double dParameter, double dValue);
    void fillDebugTooltip(QStringList* debug);

  protected:
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void paintEvent(QPaintEvent* e);
    virtual void wheelEvent(QWheelEvent* e);
    virtual void resizeEvent(QResizeEvent* e);

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
    // Pointer to pixmap of the slider
    PaintablePointer m_pSlider;
    // Pointer to pixmap of the handle
    PaintablePointer m_pHandle;
    SliderEventHandler<WSliderComposed> m_handler;

    friend class SliderEventHandler<WSliderComposed>;
};

#endif
