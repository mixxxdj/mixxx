/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EFFECTSUNITSVIEW_H
#define EFFECTSUNITSVIEW_H

#include <QtCore>
#include <QtGui>

#include "configobject.h"

class EffectsUnitsView : public QWidget
{
    Q_OBJECT

public:
    EffectsUnitsView(QWidget *parent);
    ~EffectsUnitsView();
};

#endif
