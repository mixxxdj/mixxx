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

#include "wabstractcontrol.h"
#include <qstring.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>
/**
  * A widget for a slider composed of a background pixmap and a handle.
  *
  *@author Tue & Ken Haste Andersen
  */

class WSliderComposed : public WAbstractControl {
    Q_OBJECT
  public:
    WSliderComposed(QWidget* parent = 0);
    ~WSliderComposed();
    void setup(QDomNode node);
    void setPixmaps(bool bHorizontal, const QString &filenameSlider, const QString &filenameHandle);
    inline bool isHorizontal() const { return m_bHorizontal; };

  public slots:
    void setValue(double);

  protected:
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void paintEvent(QPaintEvent*);
    virtual void wheelEvent(QWheelEvent* e);


private:
    void unsetPixmaps();

    /** Internal storage of slider position in pixels */
    int m_iPos, m_iStartHandlePos, m_iStartMousePos;
    /** Length of slider in pixels */
    int m_iSliderLength;
    /** Length of handle in pixels */
    int m_iHandleLength;
    /** True if it's a horizontal slider */
    bool m_bHorizontal;
    /** Is true if events is emitted while the slider is dragged */
    bool m_bEventWhileDrag;
    /** True if slider is dragged. Only used when m_bEventWhileDrag is false */
    bool m_bDrag;
    /** Pointer to pixmap of the slider */
    QPixmap* m_pSlider;
    /** Pointer to pixmap of the handle */
    QPixmap* m_pHandle;
};

#endif
