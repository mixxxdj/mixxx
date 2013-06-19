/**
 * @file dlgprefsounditem.cpp
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

#include "dlgprefsounditem.h"
#include "sounddevice.h"
#include "soundmanagerconfig.h"

/**
 * Constructs a new preferences sound item, representing an AudioPath and SoundDevice
 * with a label and two combo boxes.
 * @param type The AudioPathType of the path to be represented
 * @param devices The list of devices for the user to choose from (either a collection
 * of input or output devices).
 * @param isInput true if this is representing an AudioInput, false otherwise
 * @param index the index of the represented AudioPath, if applicable
 */
DlgPrefSoundItem::DlgPrefSoundItem(QWidget *parent, AudioPathType type,
        QList<SoundDevice*> &devices, bool isInput,
        unsigned int index /* = 0 */)
        : QWidget(parent)
        , m_type(type)
        , m_index(index)
        , m_devices(devices)
        , m_isInput(isInput)
        , m_savedDevice("")
        , m_savedChannel(0) {
    setupUi(this);
    if (AudioPath::isIndexed(type)) {
        typeLabel->setText(
            QString("%1 %2").arg(AudioPath::getTrStringFromType(type),
                                 QString::number(index + 1)));
    } else {
        typeLabel->setText(AudioPath::getTrStringFromType(type));
    }
    deviceComboBox->addItem(tr("None"), "None");
    connect(deviceComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(deviceChanged(int)));
    connect(channelComboBox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(settingChanged()));
    refreshDevices(m_devices);
}

DlgPrefSoundItem::~DlgPrefSoundItem() {

}

/**
 * Slot called when the parent preferences pane updates its list of sound
 * devices, to update the item widget's list of devices to display.
 */
void DlgPrefSoundItem::refreshDevices(const QList<SoundDevice*> &devices) {
    m_devices = devices;
    QString oldDev = deviceComboBox->itemData(deviceComboBox->currentIndex()).toString();
    deviceComboBox->setCurrentIndex(0);
    // not using combobox->clear means we can leave in "None" so it
    // doesn't flicker when you switch APIs... cleaner Mixxx :) bkgood
    while (deviceComboBox->count() > 1) {
        deviceComboBox->removeItem(deviceComboBox->count() - 1);
    }
    foreach (SoundDevice *device, m_devices) {
        if (!hasSufficientChannels(device)) continue;
        deviceComboBox->addItem(device->getDisplayName(), device->getInternalName());
    }
    int newIndex = deviceComboBox->findData(oldDev);
    if (newIndex != -1) {
        deviceComboBox->setCurrentIndex(newIndex);
    }
}

/**
 * Slot called when the device combo box selection changes. Updates the channel
 * combo box.
 */
void DlgPrefSoundItem::deviceChanged(int index) {
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
        unsigned char channelsForType =
            AudioPath::channelsNeededForType(m_type);
        for (unsigned int i = 1; i + (channelsForType - 1) <= numChannels; ++i) {
            if (channelsForType == 1) {
                channelComboBox->addItem(
                    QString(tr("Channel %1")).arg(i), i - 1);
            } else {
                channelComboBox->addItem(
                    QString(tr("Channels %1 - %2")).arg(QString::number(i),
                                                        QString::number(i + 1)),
                    i - 1);
            }
            // i-1 because want the data part to be what goes into audiopath's
            // channelbase which is zero-based -- bkgood
        }
    }
emitAndReturn:
    emit(settingChanged());
}

/**
 * Slot called to load the respective AudioPath from a SoundManagerConfig
 * object.
 * @note If there are multiple AudioPaths matching this instance's type
 *       and index (if applicable), then only the first one is used. A more
 *       advanced preferences pane may one day allow multiples.
 */
void DlgPrefSoundItem::loadPath(const SoundManagerConfig &config) {
    if (m_isInput) {
        QMultiHash<QString, AudioInput> inputs(config.getInputs());
        foreach (QString devName, inputs.uniqueKeys()) {
            foreach (AudioInput in, inputs.values(devName)) {
                if (in.getType() == m_type && in.getIndex() == m_index) {
                    setDevice(devName);
                    setChannel(in.getChannelGroup().getChannelBase());
                    return; // we're just using the first one found, leave
                            // multiples to a more advanced dialog -- bkgood
                }
            }
        }
    } else {
        QMultiHash<QString, AudioOutput> outputs(config.getOutputs());
        foreach (QString devName, outputs.uniqueKeys()) {
            foreach (AudioOutput out, outputs.values(devName)) {
                if (out.getType() == m_type && out.getIndex() == m_index) {
                    setDevice(devName);
                    setChannel(out.getChannelGroup().getChannelBase());
                    return; // we're just using the first one found, leave
                            // multiples to a more advanced dialog -- bkgood
                }
            }
        }
    }
    // if we've gotten here without returning, we didn't find a path applicable
    // to us so set some defaults -- bkgood
    setDevice("None"); // this will blank the channel combo box
}

/**
 * Slot called when the underlying DlgPrefSound wants this Item to
 * record its respective path with the SoundManagerConfig instance at
 * config.
 */
void DlgPrefSoundItem::writePath(SoundManagerConfig *config) const {
    SoundDevice *device = getDevice();
    if (device == NULL) {
        return;
    } // otherwise, this will have a valid audiopath
    if (m_isInput) {
        config->addInput(
                device->getInternalName(),
                AudioInput(
                    m_type,
                    channelComboBox->itemData(channelComboBox->currentIndex()).toUInt(),
                    m_index
                    )
                );
    } else {
        config->addOutput(
                device->getInternalName(),
                AudioOutput(
                    m_type,
                    channelComboBox->itemData(channelComboBox->currentIndex()).toUInt(),
                    m_index
                    )
                );
    }
}

/**
 * Slot called to tell the Item to save its selections for later use.
 */
void DlgPrefSoundItem::save() {
    m_savedDevice = deviceComboBox->itemData(deviceComboBox->currentIndex()).toString();
    m_savedChannel = channelComboBox->itemData(channelComboBox->currentIndex()).toUInt();
}

/**
 * Slot called to reload Item with previously saved settings.
 */
void DlgPrefSoundItem::reload() {
    int newDevice = deviceComboBox->findData(m_savedDevice);
    if (newDevice > -1) {
        deviceComboBox->setCurrentIndex(newDevice);
    }
    int newChannel = channelComboBox->findData(m_savedChannel);
    if (newChannel > -1) {
        channelComboBox->setCurrentIndex(newChannel);
    }
}

/**
 * Gets the currently selected SoundDevice
 * @returns pointer to SoundDevice, or NULL if the "None" option is selected.
 */
SoundDevice* DlgPrefSoundItem::getDevice() const {
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

/**
 * Selects a device in the device combo box given a SoundDevice
 * internal name, or selects "None" if the device isn't found.
 */
void DlgPrefSoundItem::setDevice(const QString &deviceName) {
    int index = deviceComboBox->findData(deviceName);
    if (index != -1) {
        deviceComboBox->setCurrentIndex(index);
    } else {
        deviceComboBox->setCurrentIndex(0); // None
    }
}

/**
 * Selects a channel in the channel combo box given a channel number,
 * or selects the first channel if the given channel isn't found.
 */
void DlgPrefSoundItem::setChannel(unsigned int channel) {
    int index = channelComboBox->findData(channel);
    if (index != -1) {
        channelComboBox->setCurrentIndex(index);
    } else {
        channelComboBox->setCurrentIndex(0); // 1
    }
}

/**
 * Checks that a given device can act as a source/input for our type.
 */
int DlgPrefSoundItem::hasSufficientChannels(const SoundDevice *device) const
{
    unsigned char needed(AudioPath::channelsNeededForType(m_type));

    if (m_isInput) {
        return device->getNumInputChannels() >= needed;
    } else {
        return device->getNumOutputChannels() >= needed;
    }
}
