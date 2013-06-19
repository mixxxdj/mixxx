/***************************************************************************
                          wlabel.h  -  description
                             -------------------
    begin                : Wed Jan 5 2005
    copyright            : (C) 2003 by Tue Haste Andersen
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

#ifndef WLABEL_H
#define WLABEL_H

#include "wwidget.h"
#include <qlabel.h>
#include <qevent.h>

/**
  *@author Tue Haste Andersen
  */

class WLabel : public WWidget {
    Q_OBJECT
  public:
    WLabel(QWidget *parent=0);
    virtual ~WLabel();
    void setup(QDomNode node);

    void setAlignment(Qt::Alignment);
    void setConstFactor(double);
    virtual QWidget* getComposedWidget() { return m_pLabel; }

  protected:
    /** Multiplication factor */
    QLabel *m_pLabel;
    QString m_qsText;
    /** Foreground and background colors */
    QColor m_qFgColor, m_qBgColor;
};

#endif
