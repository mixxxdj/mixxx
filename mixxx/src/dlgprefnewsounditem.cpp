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

DlgPrefNewSoundItem::DlgPrefNewSoundItem(QWidget *parent, QString &type,
        QList<SoundDevice*> &devices, bool isInput,
        unsigned int channelsNeeded /*= 2*/)
    : QWidget(parent)
    , m_devices(devices)
    , m_channelsNeeded(channelsNeeded)
    , m_isInput(isInput) {
    setupUi(this);
    typeLabel->setText(type);
    deviceComboBox->addItem("None" /* translate me */, "None" /*don't translate me */);
    connect(deviceComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(deviceChanged(int)));
    refreshDevices(m_devices);
}

DlgPrefNewSoundItem::~DlgPrefNewSoundItem() {

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
    // m_channelsNeeded -- bkgood
    channelComboBox->clear();
    QString selection = deviceComboBox->itemData(index).toString();
    unsigned int numChannels = 0;
    if (selection == "None") {
        return;
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
        return;
    } else {
        for (unsigned int i = 1; i + 1 <= numChannels; i += 2) {
            channelComboBox->addItem(
                QString("Channels %1 - %2").arg(i).arg(i + 1), i);
        }
    }
}
