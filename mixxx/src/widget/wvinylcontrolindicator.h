/***************************************************************************
                          wvinylcontrolindicator.h  -  description
                             -------------------
    begin                : Fri Jul 22 2003
    copyright            : (C) 2007 by Albert Santoni
    					   (C) 2003 by Tue & Ken Haste Andersen
    email                : albert [at] santoni *dot* ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WVINYLCONTROLINDICATOR_H
#define WVINYLCONTROLINDICATOR_H

#include "wwidget.h"
#include <math.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qstring.h>
#include <qtimer.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WVinylControlIndicator : public WWidget  {
   Q_OBJECT
public: 
    WVinylControlIndicator(QWidget *parent=0, const char *name=0);
    ~WVinylControlIndicator();
    void setup(QDomNode node);
    void setPixmaps(const QString &backFilename, const QString &vuFilename);
    
public slots:
    void paintEvent();
    
private:
    /** Set position number to zero and deallocate pixmaps */
    void resetPositions();

	//Timer to repaint this widget (acts independently of rest of GUI)
	QTimer* paintTimer;
    /** Current position */
    int m_iPos;
    /** Number of positions associated with this knob */
    int m_iNoPos;
    /** Associated pixmaps */
    QPixmap *m_pPixmapBack, *m_pPixmapVu, *m_pPixmapBuffer;
    /** True if it's a horizontal vu meter */
    bool m_bHorizontal;
    int iRadius; //Radius of the circular visualization.
    int iWidth;  //Width of the widget (so we know where to center the circle thingy)
    int iHeight; //Height of the widget (ditto)
    
};

#endif
