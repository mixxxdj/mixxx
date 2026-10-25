#include "soundio/sounddeviceandroidusb.h"

#include <QDebug>
#include <QtGlobal>

#include "control/controlobject.h"
#include "engine/enginemixer.h"
#include "soundio/soundmanager.h"

SoundDeviceAndroidUsb::SoundDeviceAndroidUsb(
        UserSettingsPointer config,
        SoundManager* sm,
        std::shared_ptr<mixxx::androidusb::AndroidUsbAudioDevice> pDevice)
        : SoundDevice(config, sm),
          m_pDevice(std::move(pDevice)),
          m_pFifo(std::make_unique<FIFO<CSAMPLE>>(8192 * 2)),
          m_engineBuffer(8192 * 2),
          m_renderBuffer(8192 * 2),
          m_framesPerBuffer(0),
          m_isClkRef(false),
          m_open(false),
          m_audioLatencyUsage(ConfigKey("[App]", "audio_latency_usage")),
          m_timeInAudioCallback(0),
          m_framesSinceAudioLatencyUsageUpdate(0) {
}

SoundDeviceAndroidUsb::~SoundDeviceAndroidUsb() {
    close();
}

SoundDeviceStatus SoundDeviceAndroidUsb::open(bool isClkRefDevice, int syncBuffers) {
    Q_UNUSED(syncBuffers);
    if (m_open) {
        return SoundDeviceStatus::Ok;
    }
    m_isClkRef = isClkRefDevice;

    // Start the USB device output
    const int transferMillis = m_pConfig->getValue(ConfigKey("[App]", "audio_latency"), 20);
    const int sampleRate = m_pConfig->getValue(ConfigKey("[App]", "sample_rate"), 44100);
    const int numTransfers = 4;

    if (!m_pDevice->startOutput(sampleRate, transferMillis, numTransfers, [this](uint8_t* pDestination, int frames) {
            this->render(pDestination, frames);
        })) {
        m_lastError = tr("Failed to start USB output");
        return SoundDeviceStatus::Error;
    }

    m_open = true;
    return SoundDeviceStatus::Ok;
}

bool SoundDeviceAndroidUsb::isOpen() const {
    return m_open.load();
}

SoundDeviceStatus SoundDeviceAndroidUsb::close() {
    if (!m_open) {
        return SoundDeviceStatus::Ok;
    }
    m_pDevice->stopOutput();
    m_open = false;
    return SoundDeviceStatus::Ok;
}

void SoundDeviceAndroidUsb::readProcess(SINT framesPerBuffer) {
    // This device doesn't do input currently
    Q_UNUSED(framesPerBuffer);
}

void SoundDeviceAndroidUsb::writeProcess(SINT framesPerBuffer) {
    if (!m_open) {
        return;
    }
    if (m_isClkRef) {
        processEngine();
    }
    updateAudioLatencyUsage(framesPerBuffer);
}

void SoundDeviceAndroidUsb::render(uint8_t* pDestination, int frames) {
    const int channels = m_outputs.front().channels;
    const int frameBytes = channels * sizeof(CSAMPLE);
    const int totalFrames = frames;

    // Process engine if we are the clock reference and the buffer is full
    if (m_isClkRef) {
        while (m_pFifo->size() >= totalFrames) {
            const int toRead = std::min(totalFrames, static_cast<int>(m_pFifo->size()));
            m_pFifo->read(m_engineBuffer.data(), toRead);
            for (int i = 0; i < toRead; ++i) {
                m_renderBuffer[i * channels + 0] = m_engineBuffer[i * 2 + 0];
                if (channels > 1) {
                    m_renderBuffer[i * channels + 1] = m_engineBuffer[i * 2 + 1];
                }
            }
            std::memcpy(pDestination, m_renderBuffer.data(), toRead * frameBytes);
            pDestination += toRead * frameBytes;
            frames -= toRead;
        }
    }

    // Fill remaining with silence
    if (frames > 0) {
        std::fill_n(pDestination, frames * frameBytes, 0);
    }
}

void SoundDeviceAndroidUsb::processEngine() {
    m_pEngineMixer->process(m_framesPerBuffer * 2);
    const CSAMPLE* pBuffer = m_pEngineMixer->buffer(m_outputs.front());
    m_pFifo->write(pBuffer, m_framesPerBuffer);
}

void SoundDeviceAndroidUsb::updateAudioLatencyUsage(SINT framesPerBuffer) {
    m_framesSinceAudioLatencyUsageUpdate += framesPerBuffer;
    if (m_framesSinceAudioLatencyUsageUpdate >= m_pConfig->getValue(
                                                        ConfigKey("[App]", "sample_rate"), 44100)) {
        m_audioLatencyUsage.set(static_cast<double>(m_timeInAudioCallback.count()) /
                m_framesSinceAudioLatencyUsageUpdate * 100.0);
        m_framesSinceAudioLatencyUsageUpdate = 0;
        m_timeInAudioCallback = mixxx::Duration(0);
    }
}

QString SoundDeviceAndroidUsb::getError() const {
    return m_lastError;
}

mixxx::audio::SampleRate SoundDeviceAndroidUsb::getDefaultSampleRate() const {
    return mixxx::audio::SampleRate(44100);
}
