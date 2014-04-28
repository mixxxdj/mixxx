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

#include <QLabel>

#include "widget/wlabel.h"
#include "skin/skincontext.h"

class WNumber : public WLabel  {
    Q_OBJECT
  public:
    WNumber(QWidget* pParent = NULL);
    virtual ~WNumber();

    virtual void setup(QDomNode node, const SkinContext& context);

    virtual void onConnectedControlChanged(double dParameter, double dValue);

  public slots:
    virtual void setValue(double dValue);

  protected:
    // Number of digits to round to.
    int m_iNoDigits;
};

#endif
