#pragma once

#include <QList>
#include <QString>
#include <chrono>

#include "audio/types.h"
#include "preferences/usersettings.h"
#include "soundio/sounddevicestatus.h"
#include "soundio/soundmanagerutil.h"
#include "util/types.h"

class SoundManager;
class AudioOutputBuffer;
class AudioInputBuffer;

const QString kNetworkDeviceInternalName = "Network stream";

class SoundDevice {
  public:
    SoundDevice(UserSettingsPointer config, SoundManager* sm);
    virtual ~SoundDevice() = default;

    inline const SoundDeviceId& getDeviceId() const {
        return m_deviceId;
    }
    inline const QString& getDisplayName() const {
        return m_strDisplayName;
    }
    inline const QString& getHostAPI() const {
        return m_hostAPI;
    }
    void setSampleRate(mixxx::audio::SampleRate sampleRate);
    void setConfigFramesPerBuffer(unsigned int framesPerBuffer);
    virtual SoundDeviceStatus open(bool isClkRefDevice, int syncBuffers) = 0;
    virtual bool isOpen() const = 0;
    virtual SoundDeviceStatus close() = 0;
    virtual void readProcess(SINT framesPerBuffer) = 0;
    virtual void writeProcess(SINT framesPerBuffer) = 0;
    virtual QString getError() const = 0;
    virtual mixxx::audio::SampleRate getDefaultSampleRate() const = 0;
    mixxx::audio::ChannelCount getNumOutputChannels() const;
    mixxx::audio::ChannelCount getNumInputChannels() const;
    SoundDeviceStatus addOutput(const AudioOutputBuffer& out);
    SoundDeviceStatus addInput(const AudioInputBuffer& in);
    const QList<AudioInputBuffer>& inputs() const {
        return m_audioInputs;
    }
    const QList<AudioOutputBuffer>& outputs() const {
        return m_audioOutputs;
    }

    void clearOutputs();
    void clearInputs();
    bool operator==(const SoundDevice &other) const;
    bool operator==(const QString &other) const;

    std::chrono::microseconds m_absTimeWhenPrevOutputBufferReachesDac;

  protected:
    void composeOutputBuffer(CSAMPLE* outputBuffer,
                             const SINT iFramesPerBuffer,
                             const SINT readOffset,
                             const int iFrameSize);

    void composeInputBuffer(const CSAMPLE* inputBuffer,
                            const SINT framesToPush,
                            const SINT framesWriteOffset,
                            const int iFrameSize);

    void clearInputBuffer(const SINT framesToPush,
                          const SINT framesWriteOffset);

    SoundDeviceId m_deviceId;
    UserSettingsPointer m_pConfig;
    // Pointer to the SoundManager object which we'll request audio from.
    SoundManager* m_pSoundManager;
    // The name of the soundcard, as displayed to the user
    QString m_strDisplayName;
    // The number of output channels that the soundcard has
    mixxx::audio::ChannelCount m_numOutputChannels;
    // The number of input channels that the soundcard has
    mixxx::audio::ChannelCount m_numInputChannels;
    // The current samplerate for the sound device.
    mixxx::audio::SampleRate m_sampleRate;
    // The output latency reported by portaudio streaminfo
    double m_outputLatencyMillis;
    // The name of the audio API used by this device.
    QString m_hostAPI;
    // The **configured** number of frames per buffer. We'll tell PortAudio we
    // want this many frames in a buffer, but PortAudio may still give us have a
    // differently sized buffers. As such this value should only be used for
    // configuring the audio devices. The actual runtime buffer size should be
    // used for any computations working with audio.
    SINT m_configFramesPerBuffer;
    QList<AudioOutputBuffer> m_audioOutputs;
    QList<AudioInputBuffer> m_audioInputs;
};

typedef QSharedPointer<SoundDevice> SoundDevicePointer;
