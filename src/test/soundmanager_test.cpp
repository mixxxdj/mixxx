#include "soundio/soundmanager.h"

#include <gtest/gtest.h>

#include <QHash>
#include <QList>

#include "soundio/sounddevice.h"

class MockSoundDevice : public SoundDevice {
  public:
    MockSoundDevice(const QString& name)
            : SoundDevice(nullptr, nullptr) {
        m_deviceId.name = name;
    }

    SoundDeviceStatus open(bool, int) override {
        return SoundDeviceStatus::Ok;
    }

    bool isOpen() const override {
        return true;
    }

    SoundDeviceStatus close() override {
        return SoundDeviceStatus::Ok;
    }

    void readProcess(SINT) override {
    }

    void writeProcess(SINT) override {
    }

    QString getError() const override {
        return QString();
    }

    mixxx::audio::SampleRate getDefaultSampleRate() const override {
        return mixxx::audio::SampleRate(44100);
    }
};

TEST(SoundManagerTest, SelectLocalTimeSyncRefNoSoundDevice) {
    QHash<SoundDevicePointer, QList<AudioOutput>> deviceOutputs;
    QList<SoundDevicePointer> devices;

    // No sound devices defined
    // deviceOutputs and devices are empty

    // Test case: Select local time sync reference with no sound devices
    SoundDevicePointer result = SoundManager::selectLocalTimeSyncRef(deviceOutputs, devices);
    EXPECT_EQ(result, nullptr);
}

TEST(SoundManagerTest, SelectLocalTimeSyncRefNoDeviceOutput) {
    QHash<SoundDevicePointer, QList<AudioOutput>> deviceOutputs;
    QList<SoundDevicePointer> devices;

    auto portAudioDevice1 = SoundDevicePointer(new MockSoundDevice("PortAudioDevice1"));
    auto portAudioDevice2 = SoundDevicePointer(new MockSoundDevice("PortAudioDevice2"));

    // No device outputs defined
    deviceOutputs[portAudioDevice1] = {};
    deviceOutputs[portAudioDevice2] = {};

    devices.append(portAudioDevice1);
    devices.append(portAudioDevice2);

    // Test case: Select local time sync reference with no device outputs
    SoundDevicePointer result = SoundManager::selectLocalTimeSyncRef(deviceOutputs, devices);
    EXPECT_EQ(result, nullptr);
}

TEST(SoundManagerTest, SelectLocalTimeSyncRefOneDevice) {
    QHash<SoundDevicePointer, QList<AudioOutput>> deviceOutputs;
    QList<SoundDevicePointer> devices;

    auto portAudioDevice1 = SoundDevicePointer(new MockSoundDevice("PortAudioDevice1"));
    auto portAudioDevice2 = SoundDevicePointer(new MockSoundDevice("PortAudioDevice2"));

    deviceOutputs[portAudioDevice1] = {
            AudioOutput(AudioPathType::Main,
                    0,
                    mixxx::audio::ChannelCount::stereo(),
                    0),
            AudioOutput(AudioPathType::Headphones,
                    0,
                    mixxx::audio::ChannelCount::stereo(),
                    0)};
    deviceOutputs[portAudioDevice2] = {};

    devices.append(portAudioDevice1);
    devices.append(portAudioDevice2);

    // Test case: Select local time sync reference
    SoundDevicePointer result = SoundManager::selectLocalTimeSyncRef(deviceOutputs, devices);
    EXPECT_EQ(result, portAudioDevice1);
}

TEST(SoundManagerTest, SelectLocalTimeSyncRefTwoDevices) {
    QHash<SoundDevicePointer, QList<AudioOutput>> deviceOutputs;
    QList<SoundDevicePointer> devices;

    auto portAudioDevice1 = SoundDevicePointer(new MockSoundDevice("PortAudioDevice1"));
    auto portAudioDevice2 = SoundDevicePointer(new MockSoundDevice("PortAudioDevice2"));

    deviceOutputs[portAudioDevice1] = {AudioOutput(AudioPathType::Headphones,
            0,
            mixxx::audio::ChannelCount::stereo(),
            0)};
    deviceOutputs[portAudioDevice2] = {AudioOutput(
            AudioPathType::Main, 0, mixxx::audio::ChannelCount::stereo(), 0)};

    devices.append(portAudioDevice1);
    devices.append(portAudioDevice2);

    // Test case: Select local time sync reference
    SoundDevicePointer result = SoundManager::selectLocalTimeSyncRef(deviceOutputs, devices);
    EXPECT_EQ(result, portAudioDevice1);
}

TEST(SoundManagerTest, SelectLocalTimeSyncRefWithNetworkDevice) {
    QHash<SoundDevicePointer, QList<AudioOutput>> deviceOutputs;
    QList<SoundDevicePointer> devices;

    auto portAudioDevice1 = SoundDevicePointer(new MockSoundDevice("PortAudioDevice1"));
    auto portAudioDevice2 = SoundDevicePointer(new MockSoundDevice("PortAudioDevice2"));
    auto networkDevice = SoundDevicePointer(new MockSoundDevice(kNetworkDeviceInternalName));

    deviceOutputs[portAudioDevice1] = {};
    deviceOutputs[portAudioDevice2] = {
            AudioOutput(AudioPathType::Main,
                    0,
                    mixxx::audio::ChannelCount::stereo(),
                    0),
            AudioOutput(AudioPathType::Headphones,
                    0,
                    mixxx::audio::ChannelCount::stereo(),
                    0)};
    deviceOutputs[networkDevice] = {AudioOutput(
            AudioPathType::Main, 0, mixxx::audio::ChannelCount::stereo(), 0)};

    devices.append(portAudioDevice1);
    devices.append(networkDevice);
    devices.append(portAudioDevice2);

    // Test case: Select local time sync reference
    SoundDevicePointer result = SoundManager::selectLocalTimeSyncRef(deviceOutputs, devices);
    EXPECT_EQ(result->getDeviceId(), portAudioDevice2->getDeviceId());
}
