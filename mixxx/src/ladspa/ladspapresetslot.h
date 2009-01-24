/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LADSPAPRESETSLOT_H
#define LADSPAPRESETSLOT_H

#include <QtCore>
#include <QtGui>
#include <QtXml>

class WKnob;
class WPushButton;
class WLabel;

class LADSPAPresetManager;
class LADSPAPreset;
class LADSPAPresetInstance;

typedef QVector<WKnob *> WKnobVector;
typedef QVector<QLabel *> QLabelVector;

class LADSPAPresetSlot : public QWidget
{
    Q_OBJECT

public:
    LADSPAPresetSlot(QWidget *parent, QDomElement element, int slot, LADSPAPresetManager *presetManager, QPalette m_qPalette);
    ~LADSPAPresetSlot();

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

public slots:
    void slotRemove();

private:
    WPushButton *m_pRemoveButton;
    WPushButton *m_pEnableButton;
    WKnob *m_pDryWetKnob;
    QLabel *m_pLabel;
    QLabel *m_pDryWetLabel;
    QDomElement m_qKnobElement;
    LADSPAPresetManager *m_pPresetManager;
    LADSPAPresetInstance *m_pPresetInstance;
    WKnobVector m_Knobs;
    QLabelVector m_Labels;
    int m_iKnobCount;
    QPalette m_qPalette;
    int m_iSlotNumber;
    QScrollArea *m_pScrollArea;
    QWidget *m_pScrollWidget;
    int m_iBaseWidth;

    void setPreset(LADSPAPreset *preset);
    void unsetPreset();
    void addKnob(int i);
};

#endif
