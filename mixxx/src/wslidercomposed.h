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

#include "wwidget.h"
#include <qstring.h>
#include <qpixmap.h>
/**
  * A widget for a slider composed of a background pixmap and a handle.
  *
  *@author Tue & Ken Haste Andersen
  */

class WSliderComposed : public WWidget  {
    Q_OBJECT
public: 
    WSliderComposed(QWidget *parent=0, const char *name=0);
    ~WSliderComposed();
    void setPixmaps(bool bHorizontal, const QString &filenameSlider, const QString &filenameHandle);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
public slots:
    void setValue(float);
private:
    void reset();
    void unsetPixmaps();

    /** Internal storage of slider position in pixels */
    int m_iPos;
    /** Length of slider in pixels */
    int m_iSliderLength;
    /** Length of handle in pixels */
    int m_iHandleLength;
    /** True if it's a horizontal slider */
    bool m_bHorizontal;
    /** Pointer to double buffer */
    QPixmap *m_pDoubleBuffer;
    /** Pointer to pixmap of the slider */
    QPixmap *m_pSlider;
    /** Pointer to pixmap of the handle */
    QPixmap *m_pHandle;
};

#endif
