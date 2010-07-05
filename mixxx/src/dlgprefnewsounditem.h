/**
 * @file dlgaudiopath.h
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100704
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFNEWSOUNDITEM_H
#define DLGPREFNEWSOUNDITEM_H

#include <QtCore>
#include "ui_dlgprefnewsounditem.h"

class SoundDevice;

class DlgPrefNewSoundItem : public QWidget, public Ui::DlgPrefNewSoundItem {
    Q_OBJECT;
public:
    DlgPrefNewSoundItem(QWidget *parent, QString &type, QList<SoundDevice*> &devices,
            bool isInput, unsigned int channelsNeeded = 2);
    ~DlgPrefNewSoundItem();
public slots:
    void refreshDevices(QList<SoundDevice*> &devices);
    void deviceChanged(int index);
private:
    QList<SoundDevice*> m_devices;
    unsigned int m_channelsNeeded;
    bool m_isInput;
};

#endif
