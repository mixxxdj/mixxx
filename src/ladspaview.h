/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPAVIEW_H
#define LADSPAVIEW_H

#include <QtCore>
#include <QtGui>

#include "configobject.h"

class QListWidget;
class QScrollArea;
class QGridLayout;
class WKnob;

class LADSPAPresetManager;
class LADSPAPreset;
class LADSPAPresetInstance;

class LADSPAView : public QWidget
{
    Q_OBJECT

public:
    LADSPAView(QWidget *parent);
    ~LADSPAView();

private:
    QListWidget * m_pPresetList;
    LADSPAPresetManager * m_pPresetManager;
    QWidget * m_pSlotTable;
    QGridLayout* m_pGridLayout;
};

#endif
