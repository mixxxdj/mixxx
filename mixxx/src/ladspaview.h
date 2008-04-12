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

class WKnob;

class LADSPAPresetManager;
class LADSPAPreset;
class LADSPAPresetInstance;

typedef QVector<WKnob *> WKnobVector;

class LADSPAPresetWidget : public QWidget
{
    Q_OBJECT

public:
    LADSPAPresetWidget(QWidget *parent, LADSPAPreset *preset, QDomNode node);
    ~LADSPAPresetWidget();

    void addKnob(int i, ConfigKey key, QDomNode node);

private:
    LADSPAPresetInstance * m_pInstance;
    WKnobVector m_Knobs;
};

typedef QVector<LADSPAPresetWidget *> LADSPAPresetWidgetVector;

class LADSPAView : public QWidget
{
    Q_OBJECT

public:
    LADSPAView(QWidget *parent);
    ~LADSPAView();

public slots:
    void slotSelect();

private:
    QListWidget * m_pPresetList;
    int m_iHeight;
    int m_iWidth;
    int m_iListWidth;
    int m_iScrollAreaWidth;
    LADSPAPresetManager * m_pPresetManager;
    LADSPAPresetWidget * m_pWidget;
    QDomNode m_qKnobSkinNode;
    QScrollArea * m_pScrollArea;
    QWidget * m_pScrollAreaWidget;
};

#endif
