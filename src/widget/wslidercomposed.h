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
    void setSliderPixmap(const QString& filenameSlider);
    void setHandlePixmap(bool bHorizontal, const QString& filenameHandle);
    inline bool isHorizontal() const { return m_bHorizontal; };
  public slots:
    void onConnectedControlValueChanged(double);
    void fillDebugTooltip(QStringList* debug);

  protected:
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void paintEvent(QPaintEvent* e);
    virtual void wheelEvent(QWheelEvent* e);

  private:
    void unsetPixmaps();

    double m_dOldValue;
    // True if right mouse button is pressed.
    bool m_bRightButtonPressed;
    // Internal storage of slider position in pixels
    int m_iPos, m_iStartHandlePos, m_iStartMousePos;
    // Length of handle in pixels
    int m_iHandleLength;
    // True if it's a horizontal slider
    bool m_bHorizontal;
    // Is true if events is emitted while the slider is dragged
    bool m_bEventWhileDrag;
    // True if slider is dragged. Only used when m_bEventWhileDrag is false
    bool m_bDrag;
    // Pointer to pixmap of the slider
    PaintablePointer m_pSlider;
    // Pointer to pixmap of the handle
    PaintablePointer m_pHandle;
};

#endif
