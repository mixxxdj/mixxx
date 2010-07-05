/**
 * @file dlgprefnewsounditem.cpp
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

#include "dlgprefnewsounditem.h"
#include "sounddevice.h"

DlgPrefNewSoundItem::DlgPrefNewSoundItem(QWidget *parent, AudioPath::AudioPathType type,
        QList<SoundDevice*> &devices, bool isInput,
        unsigned int index /* = 0 */)
    : QWidget(parent)
    , m_type(type)
    , m_index(index)
    , m_devices(devices)
    , m_isInput(isInput) {
    setupUi(this);
    if (AudioPath::isIndexable(type)) {
        typeLabel->setText(
            QString("%1 %2").arg(AudioPath::getStringFromType(type)).arg(index + 1));
    } else {
        typeLabel->setText(AudioPath::getStringFromType(type));
    }
    deviceComboBox->addItem("None" /* translate me */, "None" /*don't translate me */);
    connect(deviceComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(deviceChanged(int)));
    connect(channelComboBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(settingChanged()));
    refreshDevices(m_devices);
}

DlgPrefNewSoundItem::~DlgPrefNewSoundItem() {

}

SoundDevice *DlgPrefNewSoundItem::getDevice() const {
    QString selection = deviceComboBox->itemData(deviceComboBox->currentIndex()).toString();
    if (selection == "None") {
        return NULL;
    }
    foreach (SoundDevice *device, m_devices) {
        if (selection == device->getInternalName()) {
            return device;
        }
    }
    // looks like something became invalid ???
    deviceComboBox->setCurrentIndex(0); // set it to none
    return NULL;
}

AudioPath DlgPrefNewSoundItem::getPath() const {
    unsigned int channelBase = 0;
    if (channelComboBox->count() > 0) {
        channelBase = channelComboBox->itemData(channelComboBox->currentIndex()).toUInt();
    }
    if (m_isInput) {
        return AudioReceiver(m_type, channelBase, m_index);
    } else {
        return AudioSource(m_type, channelBase, m_index);
    }
}

void DlgPrefNewSoundItem::refreshDevices(QList<SoundDevice*> &devices) {
    m_devices = devices;
    deviceComboBox->setCurrentIndex(0);
    // not using combobox->clear means we can leave in "None" so it
    // doesn't flicker when you switch APIs... cleaner Mixxx :) bkgood
    while (deviceComboBox->count() > 1) {
        deviceComboBox->removeItem(deviceComboBox->count() - 1);
    }
    foreach (SoundDevice *device, m_devices) {
        deviceComboBox->addItem(device->getDisplayName(), device->getInternalName());
    }
}

void DlgPrefNewSoundItem::deviceChanged(int index) {
    // TODO assumes we're looking for a two-channel device -- eventually will want
    // to give the option of mono for a microphone, depending on the value of
    // m_type -- bkgood
    channelComboBox->clear();
    QString selection = deviceComboBox->itemData(index).toString();
    unsigned int numChannels = 0;
    if (selection == "None") {
        goto emitAndReturn;
    } else {
        foreach (SoundDevice *device, m_devices) {
            if (device->getInternalName() == selection) {
                if (m_isInput) {
                    numChannels = device->getNumInputChannels();
                } else {
                    numChannels = device->getNumOutputChannels();
                }
            }
        }
    }
    if (numChannels == 0) {
        goto emitAndReturn;
    } else {
        for (unsigned int i = 1; i + 1 <= numChannels; i += 2) {
            channelComboBox->addItem(
                QString("Channels %1 - %2").arg(i).arg(i + 1), i - 1);
            // i-1 because want the data part to be what goes into audiopath's
            // channelbase which is zero-based -- bkgood
        }
    }
emitAndReturn:
    emit(settingChanged());
}
