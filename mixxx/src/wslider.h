/***************************************************************************
                          wslider.h  -  description
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

#ifndef WSLIDER_H
#define WSLIDER_H

#include <qwidget.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WSlider : public QWidget  {
   Q_OBJECT
public: 
	WSlider(QWidget *parent=0, const char *name=0);
	~WSlider();
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);
public slots:
    void setValue(int);
    void slotSetPosition(int);
signals:
    void valueChanged(int);
private:
    void setDefaultValue();

    /** Internal representation of slider value */
    int value;

    /** Internal storage of slider position in pixels */
    int pos;

    /** Used internally to represent state of slider (size and orientation) */
    int size_state;

    /** Length of slider in pixels */
    int slider_length;

    /** Length of handle in pixels */
    int handle_length;

    /** Pixmaps used to draw slider */
    static QPixmap *smallv, *smallv_h, *midv, *midv_h, *largeh, *largeh_h;
};

#endif
