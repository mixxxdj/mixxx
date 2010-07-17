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
#include "audiopath.h"

class SoundDevice;
class SoundManagerConfig;

class DlgPrefNewSoundItem : public QWidget, public Ui::DlgPrefNewSoundItem {
    Q_OBJECT;
public:
    DlgPrefNewSoundItem(QWidget *parent, AudioPathType type,
            QList<SoundDevice*> &devices, bool isInput, unsigned int index = 0);
    ~DlgPrefNewSoundItem();
signals:
    void settingChanged();
public slots:
    void refreshDevices(const QList<SoundDevice*> &devices);
    void deviceChanged(int index);
    void loadPath(const SoundManagerConfig &config);
    void writePath(SoundManagerConfig *config) const;
private:
    SoundDevice* getDevice() const; // if this returns NULL, we don't have a valid AudioPath
    void setDevice(const SoundDevice *device);
    void setChannel(unsigned int channel);
    AudioPathType m_type;
    unsigned int m_index;
    QList<SoundDevice*> m_devices;
    bool m_isInput;
};

#endif
