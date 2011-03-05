/***************************************************************************
                          wnumber.h  -  description
                             -------------------
    begin                : Wed Jun 18 2003
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

#ifndef WNUMBER_H
#define WNUMBER_H

#include "wwidget.h"
#include <qlabel.h>
#include <qevent.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class WNumber : public WWidget  {
    Q_OBJECT
public:
    WNumber(QWidget *parent=0);
    virtual ~WNumber();
    void setup(QDomNode node);
    void setNumDigits(int);
    void setConstFactor(double);

public slots:
    void setValue(double dValue);

protected:
    QLabel *m_pLabel;
    QString m_qsText;
    int m_iNoDigits;
    /** Foreground and background colors */
    QColor m_qFgColor, m_qBgColor;
    /** Constant factor added to value */
    double m_dConstFactor;
};

#endif
