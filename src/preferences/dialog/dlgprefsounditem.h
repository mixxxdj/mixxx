#pragma once

#include "preferences/dialog/ui_dlgprefsounditem.h"
#include "soundio/soundmanagerutil.h"
#include "soundio/sounddevice.h"

class SoundManagerConfig;

/**
 * Class representing an input or output selection widget in DlgPrefSound.
 * The widget includes a label describing the input or output, a combo box
 * with a list of available devices and a combo box with a list of available
 * channels.
 */
class DlgPrefSoundItem : public QWidget, public Ui::DlgPrefSoundItem {
    Q_OBJECT
  public:
    DlgPrefSoundItem(QWidget* parent, AudioPathType type,
            const QList<SoundDevicePointer>& devices,
            bool isInput, unsigned int index = 0);
    virtual ~DlgPrefSoundItem();

    AudioPathType type() const { return m_type; };
    unsigned int index() const { return m_index; };
    bool isInput() {
        return m_isInput;
    }
    const SoundDeviceId getDeviceId() {
        return deviceComboBox->itemData(deviceComboBox->currentIndex()).value<SoundDeviceId>();
    }
    int getChannelIndex() {
        return channelComboBox->currentIndex();
    }
    void selectFirstUnusedChannelIndex(const QList<int>& selectedChannels);

    int getLatencyOffsetMs() const {
        return m_latencyOffsetMs;
    }
    void setLatencyOffsetMs(int ms) {
        m_latencyOffsetMs = ms;
    }

  signals:
     void selectedDeviceChanged();
     void selectedChannelsChanged();
     void configuredDeviceNotFound();
+    void removeClicked();

   private slots:
    void refreshDevices(const QList<SoundDevicePointer>& devices);
    void deviceChanged(int index);
    void channelChanged();
    void loadPath(const SoundManagerConfig& config);
    void writePath(SoundManagerConfig *config) const;
    void save();
    void reload();

  private:
    SoundDevicePointer getDevice() const; // if this returns NULL, we don't have a valid AudioPath
    void setDevice(const SoundDeviceId& device);
    void setChannel(unsigned int channelBase, unsigned int channels);
    int hasSufficientChannels(const SoundDevice& device) const;

    AudioPathType m_type;
    unsigned int m_index;
    QList<SoundDevicePointer> m_devices;
    bool m_isInput;
    SoundDeviceId m_savedDevice;
    // Because QVariant supports QPoint natively we use a QPoint to store the
    // channel info. x is the channel base and y is the channel count.
    QPoint m_savedChannel;
    bool m_emitSettingChanged;
    int m_latencyOffsetMs = 0;
    int m_savedLatencyOffsetMs = 0;
};
