#include "preferences/dialog/dlgprefsounditem.h"

#include <QPoint>

#include "moc_dlgprefsounditem.cpp"
#include "soundio/sounddevice.h"
#include "soundio/soundmanagerconfig.h"

/// Constructs a new preferences sound item, representing an AudioPath and SoundDevice
/// with a label and two combo boxes.
/// @param type The AudioPathType of the path to be represented
/// @param devices The list of devices for the user to choose from (either a collection
/// of input or output devices).
/// @param isInput true if this is representing an AudioInput, false otherwise
/// @param index the index of the represented AudioPath, if applicable
DlgPrefSoundItem::DlgPrefSoundItem(
        QWidget* parent,
        AudioPathType type,
        const QList<SoundDevicePointer>& devices,
        bool isInput,
        unsigned int index)
        : QWidget(parent),
          m_type(type),
          m_index(index),
          m_isInput(isInput),
          m_emitSettingChanged(true) {
    setupUi(this);
    typeLabel->setText(AudioPath::getTrStringFromType(type, index));

    deviceComboBox->addItem(SoundManagerConfig::kEmptyComboBox,
            QVariant::fromValue(SoundDeviceId()));

    connect(deviceComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSoundItem::deviceChanged);
    connect(channelComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSoundItem::channelChanged);
    refreshDevices(devices);
}

DlgPrefSoundItem::~DlgPrefSoundItem() {

}

/// Slot called when the parent preferences pane updates its list of sound
/// devices, to update the item widget's list of devices to display.
void DlgPrefSoundItem::refreshDevices(const QList<SoundDevicePointer>& devices) {
    m_devices = devices;
    SoundDeviceId oldDev = deviceComboBox->itemData(deviceComboBox->currentIndex()).value<SoundDeviceId>();
    deviceComboBox->setCurrentIndex(0);
    // not using combobox->clear means we can leave in "None" so it
    // doesn't flicker when you switch APIs... cleaner Mixxx :) bkgood
    while (deviceComboBox->count() > 1) {
        deviceComboBox->removeItem(deviceComboBox->count() - 1);
    }
    for (const auto& pDevice : std::as_const(m_devices)) {
        if (!hasSufficientChannels(*pDevice)) {
            continue;
        }
        deviceComboBox->addItem(pDevice->getDisplayName(), QVariant::fromValue(pDevice->getDeviceId()));
    }
    int newIndex = deviceComboBox->findData(QVariant::fromValue(oldDev));
    if (newIndex != -1) {
        deviceComboBox->setCurrentIndex(newIndex);
    }
}

/// Slot called when the device combo box selection changes. Updates the channel
/// combo box.
void DlgPrefSoundItem::deviceChanged(int index) {
    channelComboBox->clear();
    SoundDeviceId selection = deviceComboBox->itemData(index).value<SoundDeviceId>();
    mixxx::audio::ChannelCount numChannels;
    if (selection == SoundDeviceId()) {
        goto emitAndReturn;
    } else {
        for (const auto& pDevice : std::as_const(m_devices)) {
            if (pDevice->getDeviceId() == selection) {
                if (m_isInput) {
                    numChannels = pDevice->getNumInputChannels();
                } else {
                    numChannels = pDevice->getNumOutputChannels();
                }
            }
        }
    }
    if (!numChannels.isValid()) {
        goto emitAndReturn;
    } else {
        mixxx::audio::ChannelCount minChannelsForType =
                AudioPath::minChannelsForType(m_type);
        mixxx::audio::ChannelCount maxChannelsForType =
                AudioPath::maxChannelsForType(m_type);

        channelComboBox->blockSignals(true);
        // Count down from the max so that stereo channels are first.
        for (int channelsForType = maxChannelsForType;
                 channelsForType >= minChannelsForType; --channelsForType) {
            for (unsigned int i = 1; i + (channelsForType - 1) <= numChannels;
                     i += channelsForType) {
                QString channelString;
                if (channelsForType == 1) {
                    channelString = tr("Channel %1").arg(i);
                } else {
                    channelString = tr("Channels %1 - %2").arg(
                            QString::number(i),
                            QString::number(i + channelsForType - 1));
                }

                // Because QComboBox supports QPoint natively (via QVariant) we
                // use a QPoint to store the channel info. x is the channel base
                // and y is the channel count. We use i - 1 because the channel
                // base is 0-indexed.
                channelComboBox->addItem(channelString,
                                         QPoint(i - 1, channelsForType));
            }
        }
        channelComboBox->setCurrentIndex(-1); // clear selection
        channelComboBox->blockSignals(false);
    }
emitAndReturn:
    if (m_emitSettingChanged) {
        emit selectedDeviceChanged();
    }
}

void DlgPrefSoundItem::channelChanged() {
    if (m_emitSettingChanged) {
        emit selectedChannelsChanged();
    }
}

void DlgPrefSoundItem::selectFirstUnusedChannelIndex(const QList<int>& selectedChannels) {
    // Go through the list of occupied channel indices and pick the first unoccupied
    for (int i = 0; i < channelComboBox->count(); i++) {
        if (!selectedChannels.contains(i)) {
            // TODO(xxx) Some ideas to improve auto-select:
            // * check selected indices and new selection for channel overlap, e.g.
            //   if the device has 4 channels and ch.1/2 + ch.3/4 are already selected
            //   don't select next index (ch.1) but fall back to ch.1/2?
            // * if there are only mono indices selected, try to pick the next mono
            //   channel index instead of suggesting index 0 (ch.1/)
            channelComboBox->setCurrentIndex(i);
            return;
        }
    }
}

/// Slot called to load the respective AudioPath from a SoundManagerConfig
/// object.
/// @note If there are multiple AudioPaths matching this instance's type
///       and index (if applicable), then only the first one is used. A more
///       advanced preferences pane may one day allow multiples.
void DlgPrefSoundItem::loadPath(const SoundManagerConfig &config) {
    if (m_isInput) {
        const auto inputDeviceMap = config.getInputs();
        for (auto it = inputDeviceMap.cbegin(); it != inputDeviceMap.cend(); ++it) {
            if (it.value().getType() == m_type && it.value().getIndex() == m_index) {
                setDevice(it.key());
                setChannel(it.value().getChannelGroup().getChannelBase(),
                            it.value().getChannelGroup().getChannelCount());
                return;
            }
        }
    } else {
        const auto ouputDeviceMap = config.getOutputs();
        for (auto it = ouputDeviceMap.cbegin(); it != ouputDeviceMap.cend(); ++it) {
            if (it.value().getType() == m_type && it.value().getIndex() == m_index) {
                setDevice(it.key());
                setChannel(it.value().getChannelGroup().getChannelBase(),
                            it.value().getChannelGroup().getChannelCount());
                return;
            }
        }
    }
    setDevice(SoundDeviceId()); // this will blank the channel combo box
}

/// Slot called when the underlying DlgPrefSound wants this Item to
/// record its respective path with the SoundManagerConfig instance at
/// config.
void DlgPrefSoundItem::writePath(SoundManagerConfig* config) const {
    SoundDevicePointer pDevice = getDevice();
    if (!pDevice) {
        return;
    } // otherwise, this will have a valid audiopath

    // Because QComboBox supports QPoint natively (via QVariant) we use a QPoint
    // to store the channel info. x is the channel base and y is the channel
    // count.
    QPoint channelData = channelComboBox->itemData(
        channelComboBox->currentIndex()).toPoint();
    int channelBase = channelData.x();
    const auto channelCount = mixxx::audio::ChannelCount(channelData.y());

    // check config for occupied channels of this device
    // auto-select next free channel (pair)

    if (m_isInput) {
        config->addInput(
                pDevice->getDeviceId(),
                AudioInput(m_type, channelBase, channelCount, m_index));
    } else {
        config->addOutput(
                pDevice->getDeviceId(),
                AudioOutput(m_type, channelBase, channelCount, m_index));
    }
}

/// Slot called to tell the Item to save its selections for later use.
void DlgPrefSoundItem::save() {
    m_savedDevice = deviceComboBox->itemData(deviceComboBox->currentIndex()).value<SoundDeviceId>();
    m_savedChannel = channelComboBox->itemData(channelComboBox->currentIndex()).toPoint();
}

/// Slot called to reload Item with previously saved settings.
void DlgPrefSoundItem::reload() {
    int newDevice = deviceComboBox->findData(QVariant::fromValue(m_savedDevice));
    if (newDevice > -1) {
        deviceComboBox->setCurrentIndex(newDevice);
    }
    int newChannel = channelComboBox->findData(m_savedChannel);
    if (newChannel > -1) {
        channelComboBox->setCurrentIndex(newChannel);
    }
}

/// Gets the currently selected SoundDevice
/// @returns pointer to SoundDevice, or NULL if the "None" option is selected.
SoundDevicePointer DlgPrefSoundItem::getDevice() const {
    SoundDeviceId selection = deviceComboBox->itemData(deviceComboBox->currentIndex()).value<SoundDeviceId>();
    if (selection == SoundDeviceId()) {
        return SoundDevicePointer();
    }
    for (const auto& pDevice : std::as_const(m_devices)) {
        if (selection == pDevice->getDeviceId()) {
            //qDebug() << "DlgPrefSoundItem::getDevice" << pDevice->getDeviceId();
            return pDevice;
        }
    }
    // looks like something became invalid ???
    deviceComboBox->setCurrentIndex(0); // set it to none
    return SoundDevicePointer();
}

/// Selects a device in the device combo box given a SoundDevice' internal name,
/// or selects "None" if the device is nullptr or isn't found.
/// Called only internally via DlPrefSound::loadPaths()
void DlgPrefSoundItem::setDevice(const SoundDeviceId& device) {
    int index = deviceComboBox->findData(QVariant::fromValue(device));
    if (index == -1) {
        deviceComboBox->setCurrentIndex(0); // None
        emit selectedDeviceChanged();
        if (device != SoundDeviceId()) {
            // Notify DlgPrefSound that the device that can't be found.
            emit configuredDeviceNotFound();
        }
    } else {
        m_emitSettingChanged = false;
        deviceComboBox->setCurrentIndex(index);
        m_emitSettingChanged = true;
    }
}

/// Selects a channel in the channel combo box given a channel number,
/// or selects the first channel if the given channel isn't found.
void DlgPrefSoundItem::setChannel(unsigned int channelBase,
                                  unsigned int channels) {
    // Because QComboBox supports QPoint natively (via QVariant) we use a QPoint
    // to store the channel info. x is the channel base and y is the channel
    // count.
    int index = channelComboBox->findData(QPoint(channelBase, channels));
    if (index == -1) {
        // channel(s) not found
        channelComboBox->setCurrentIndex(0); // 1
        emit selectedChannelsChanged();
    } else {
        m_emitSettingChanged = false;
        channelComboBox->setCurrentIndex(index);
        m_emitSettingChanged = true;
    }
}

/// Checks that a given device can act as a source/input for our type.
int DlgPrefSoundItem::hasSufficientChannels(const SoundDevice& device) const {
    const auto needed = AudioPath::minChannelsForType(m_type);

    if (m_isInput) {
        return device.getNumInputChannels() >= needed;
    } else {
        return device.getNumOutputChannels() >= needed;
    }
}
