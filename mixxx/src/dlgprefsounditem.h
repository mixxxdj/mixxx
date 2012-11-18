/**
 * @file dlgprefsounditem.h
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

#ifndef DLGPREFSOUNDITEM_H
#define DLGPREFSOUNDITEM_H

#include <QtCore>
#include "ui_dlgprefsounditem.h"
#include "soundmanagerutil.h"

class SoundDevice;
class SoundManagerConfig;

/**
 * Class representing an input or output selection widget in DlgPrefSound.
 * The widget includes a label describing the input or output, a combo box
 * with a list of available devices and a combo box with a list of available
 * channels.
 */
class DlgPrefSoundItem : public QWidget, public Ui::DlgPrefSoundItem {
    Q_OBJECT;
public:
    DlgPrefSoundItem(QWidget *parent, AudioPathType type,
            QList<SoundDevice*> &devices, bool isInput, unsigned int index = 0);
    ~DlgPrefSoundItem();
    AudioPathType type() const { return m_type; };
    unsigned int index() const { return m_index; };
signals:
    void settingChanged();
public slots:
    void refreshDevices(const QList<SoundDevice*> &devices);
    void deviceChanged(int index);
    void loadPath(const SoundManagerConfig &config);
    void writePath(SoundManagerConfig *config) const;
    void save();
    void reload();
private:
    SoundDevice* getDevice() const; // if this returns NULL, we don't have a valid AudioPath
    void setDevice(const QString &deviceName);
    void setChannel(unsigned int channel);
    int hasSufficientChannels(const SoundDevice *device) const;
    AudioPathType m_type;
    unsigned int m_index;
    QList<SoundDevice*> m_devices;
    bool m_isInput;
    QString m_savedDevice;
    unsigned int m_savedChannel;
};

#endif
