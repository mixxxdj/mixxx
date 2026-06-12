#include "soundio/sounddevicepipewire.h"

#include <float.h>
#include <pipewire/pipewire.h>
#include <spa/node/io.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/audio/raw.h>
#include <spa/param/latency-utils.h>
#include <spa/param/latency.h>
#include <spa/param/param.h>
#include <spa/utils/defs.h>
#include <spa/utils/result.h>

#include <cstring>

#include "soundio/pipewireenumerator.h"
#include "util/assert.h"
#include "util/types.h"

#if __has_include(<valgrind/valgrind.h>)
#include <valgrind/valgrind.h>
#endif

#include <QRegularExpression>
#include <QThread>
#include <QtDebug>
#include <cstddef>

#include "control/controlobject.h"
#include "sounddevicenetwork.h"
#include "soundio/sounddevice.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"
#include "util/defs.h"
#include "util/denormalsarezero.h"
#include "util/fifo.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/timer.h"
#include "util/trace.h"
#include "waveform/visualplayposition.h"

namespace {

// Buffer for drift correction 1 full, 1 for r/w, 1 empty
constexpr int kDriftReserve = 1;

// Buffer for drift correction 1 full, 1 for r/w, 1 empty
constexpr int kFifoSize = 2 * kDriftReserve + 1;

constexpr int kCpuUsageUpdateRate = 30; // in 1/s, fits to display frame rate

// We warn only at invalid timing 3, since the first two
// callbacks can be always wrong due to a setup/open jitter
constexpr int m_invalidTimeInfoWarningCount = 3;

const QRegularExpression kAlsaHwDeviceRegex("(.*) \\((plug)?(hw:(\\d)+(,(\\d)+))?\\)");

const QString kAppGroup = QStringLiteral("[App]");
} // anonymous namespace

SoundDevicePipewire::SoundDevicePipewire(UserSettingsPointer config,
        SoundManager* sm,
        PipewireEnumerator* enumerator,
        // PipewireEnumerator *enumerator,
        uint32_t id,
        const std::string_view name)
        : SoundDevice(config, sm),
          m_pFilter(nullptr),
          m_outputDrift(false),
          m_inputDrift(false),
          m_bSetThreadPriority(false),
          m_audioLatencyUsage(kAppGroup, QStringLiteral("audio_latency_usage")),
          m_framesSinceAudioLatencyUsageUpdate(0),
          m_syncBuffers(2),
          m_invalidTimeInfoCount(0),
          m_lastCallbackEntrytoDacSecs(0),
          m_pEnumerator(enumerator),
          m_closing(false) {
    m_hostAPI = "PipeWire";
    m_deviceId.name = name.data();

    qWarning() << "PipeWire device: " << name.data();

    m_deviceId.deviceIndex = id;
    m_strDisplayName = QString::fromUtf8(name);
    m_numInputChannels = mixxx::audio::ChannelCount(0);
    m_numOutputChannels = mixxx::audio::ChannelCount(0);
}

SoundDevicePipewire::~SoundDevicePipewire() {
    if (isOpen()) {
        close();
    }
}

void SoundDevicePipewire::registerDevicePort(uint32_t id, const struct spa_dict* props) {
    std::string name = spa_dict_lookup(props, PW_KEY_PORT_ALIAS);
    const char* dir = spa_dict_lookup(props, PW_KEY_PORT_DIRECTION);

    if (name.empty()) {
        name = spa_dict_lookup(props, PW_KEY_PORT_NAME);
    }

    if (name.empty()) {
        name = dir;
        name += ":";
        name += spa_dict_lookup(props, PW_KEY_PORT_ID);
    }

    if (strcmp(dir, "in") == 0) {
        m_inDevicePorts.emplace_back(id, name);
        uint size = m_inDevicePorts.size();
        m_numOutputChannels = mixxx::audio::ChannelCount::fromInt(size);

        qDebug() << "PipeWire: " << m_deviceId.name << " in ports: " << m_numOutputChannels.value();

        if (isOpen()) {
            pw_thread_loop* pLoop = m_pEnumerator->getThreadLoop();
            pw_thread_loop_lock(pLoop);
            m_outPorts.emplace_back(PW_ID_ANY, addFilterPort(SPA_DIRECTION_OUTPUT, size - 1));
            pw_thread_loop_unlock(pLoop);
        }

    } else if (strcmp(dir, "out") == 0) {
        m_outDevicePorts.emplace_back(id, name);
        uint size = m_outDevicePorts.size();
        m_numInputChannels = mixxx::audio::ChannelCount::fromInt(size);

        qDebug() << "PipeWire: " << m_deviceId.name << " out ports: " << m_numInputChannels.value();

        if (isOpen()) {
            pw_thread_loop* pLoop = m_pEnumerator->getThreadLoop();
            pw_thread_loop_lock(pLoop);
            m_inPorts.emplace_back(PW_ID_ANY, addFilterPort(SPA_DIRECTION_INPUT, size - 1));
            pw_thread_loop_unlock(pLoop);
        }
    }
}

void SoundDevicePipewire::unregisterDevicePort(uint32_t id, spa_direction direction) {
    switch (direction) {
    case SPA_DIRECTION_INPUT:
        m_inDevicePorts.erase(m_inDevicePorts.begin() + id);
        m_numInputChannels = mixxx::audio::ChannelCount::fromInt(m_inDevicePorts.size());
        break;
    case SPA_DIRECTION_OUTPUT:
        m_outDevicePorts.erase(m_outDevicePorts.begin() + id);
        m_numOutputChannels = mixxx::audio::ChannelCount::fromInt(m_outDevicePorts.size());
        break;
    }
}

SoundDeviceStatus SoundDevicePipewire::open(bool isClkRefDevice, int syncBuffers) {
    m_closing = false;
    m_isClkRef = isClkRefDevice;

    if (m_audioOutputs.empty() && m_audioInputs.empty()) {
        m_lastError = QStringLiteral(
                "No inputs or outputs in SDPA::open() "
                "(THIS IS A BUG, this should be filtered by SM::setupDevices)");
        return SoundDeviceStatus::Error;
    }

    size_t numInputs = 0;
    size_t numOutputs = 0;

    // Look at how many audio outputs we have,
    // so we can figure out how many output channels we need to open.

    foreach (AudioOutput out, m_audioOutputs) {
        ChannelGroup channelGroup = out.getChannelGroup();
        uint8_t highChannel = channelGroup.getChannelBase() + channelGroup.getChannelCount();
        numOutputs = std::max<size_t>(numOutputs, highChannel);
    }

    foreach (AudioInput in, m_audioInputs) {
        ChannelGroup channelGroup = in.getChannelGroup();
        uint8_t highChannel = channelGroup.getChannelBase() + channelGroup.getChannelCount();
        numInputs = std::max<size_t>(numInputs, highChannel);
    }

    numInputs = std::min(numInputs, m_outDevicePorts.size());
    numOutputs = std::min(numOutputs, m_inDevicePorts.size());

    // Sample rate
    if (!m_sampleRate.isValid()) {
        m_sampleRate = SoundManagerConfig::kMixxxDefaultSampleRate;
    }

    SINT framesPerBuffer = m_configFramesPerBuffer;

    uint64_t bufferMSec = framesPerBuffer / m_sampleRate.value() * 1000;
    qDebug() << "Requested sample rate: " << m_sampleRate << "Hz and buffer size:"
             << bufferMSec << "ms";

    qDebug() << "Output channels:" << numOutputs
             << "| Input channels:" << numInputs;

    qDebug() << "Opening stream with id" << m_deviceId.deviceIndex;

    m_lastCallbackEntrytoDacSecs = bufferMSec / 1000;

    m_syncBuffers = syncBuffers;

    // Create the callback function pointer.
    const pw_filter_events* pEvents = nullptr;

    pw_thread_loop* pLoop = m_pEnumerator->getThreadLoop();
    pw_thread_loop_lock(pLoop);

    pw_properties* props = pw_properties_new(PW_KEY_MEDIA_TYPE,
            "Audio",
            PW_KEY_MEDIA_CATEGORY,
            "Filter",
            PW_KEY_MEDIA_ROLE,
            "DSP",
            // the target is to have this enabled and drift correction work fine
            // "resample.disable", "true",
            nullptr);

    pw_properties_setf(props, PW_KEY_NODE_RATE, "1/%u", m_sampleRate.value());
    pw_properties_setf(props, PW_KEY_NODE_LATENCY, "%td/%u", framesPerBuffer, m_sampleRate.value());

    m_pFilter = pw_filter_new(m_pEnumerator->getCore(), "mixxx", props);

    // currently this is similar to PortAudio sync settings
    // but its planned to revamp this, and take the buffering
    // to PipewireEnumerator, at soundcard level instead of node level
    if (m_isClkRef) {
        m_fifoSize = 0;
        pEvents = &filter_clkref_events;
        for (size_t i = 0; i < numOutputs; i++) {
            m_outPorts.emplace_back(PW_ID_ANY, addFilterPort(SPA_DIRECTION_OUTPUT, i));
        }
        for (size_t i = 0; i < numInputs; i++) {
            m_inPorts.emplace_back(PW_ID_ANY, addFilterPort(SPA_DIRECTION_INPUT, i));
        }
    } else if (m_syncBuffers == 2) { // "Default (long delay)"
        m_fifoSize = framesPerBuffer * kFifoSize;
        pEvents = &filter_drift_events;
        // to avoid overflows when one callback overtakes the other or
        // when there is a clock drift compared to the clock reference device
        // we need an additional artificial delay
        for (size_t i = 0; i < numOutputs; i++) {
            m_outPorts.emplace_back(PW_ID_ANY, addFilterPort(SPA_DIRECTION_OUTPUT, i));
            int writeCount = framesPerBuffer * kFifoSize / 2;
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;
            (void)m_outPorts[i].fifo->aquireWriteRegions(
                    writeCount, &dataPtr1, &size1, &dataPtr2, &size2);
            SampleUtil::clear(dataPtr1, size1);
            if (size2 > 0) {
                SampleUtil::clear(dataPtr2, size2);
            }
            m_outPorts[i].fifo->releaseWriteRegions(writeCount);
        }
        for (size_t i = 0; i < numInputs; i++) {
            m_inPorts.emplace_back(PW_ID_ANY, addFilterPort(SPA_DIRECTION_INPUT, i));
            int writeCount = framesPerBuffer * kFifoSize / 2;
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;
            (void)m_inPorts[i].fifo->aquireWriteRegions(
                    writeCount, &dataPtr1, &size1, &dataPtr2, &size2);
            SampleUtil::clear(dataPtr1, size1);
            SampleUtil::clear(dataPtr2, size2);
            m_inPorts[i].fifo->releaseWriteRegions(writeCount);
        }
    } else if (m_syncBuffers == 1) { // "Disabled (short delay)"
        // this can be used on a second device when it is driven by the Clock
        // reference device clock
        pEvents = &filter_events;
        m_fifoSize = framesPerBuffer;
        for (size_t i = 0; i < numOutputs; i++) {
            m_outPorts.emplace_back(PW_ID_ANY, addFilterPort(SPA_DIRECTION_OUTPUT, i));
        }
        for (size_t i = 0; i < numInputs; i++) {
            m_inPorts.emplace_back(PW_ID_ANY, addFilterPort(SPA_DIRECTION_INPUT, i));
        }
    } else if (m_syncBuffers == 0) { // "Experimental (no delay)"
                                     // deal with zero buffering.
                                     // pw_filter_get_dsp_buffer should work on same subgraphs,
                                     // on different subgraphs, buffer samples
        pEvents = &filter_events;
        m_fifoSize = framesPerBuffer * 2;
        for (size_t i = 0; i < numOutputs; i++) {
            m_outPorts.emplace_back(PW_ID_ANY, addFilterPort(SPA_DIRECTION_OUTPUT, i));
        }
        for (size_t i = 0; i < numInputs; i++) {
            m_inPorts.emplace_back(PW_ID_ANY, addFilterPort(SPA_DIRECTION_INPUT, i));
        }
    }

    spa_zero(m_listener);
    pw_filter_add_listener(m_pFilter, &m_listener, pEvents, this);

    int res = pw_filter_connect(m_pFilter,
            PW_FILTER_FLAG_RT_PROCESS,
            // &params,
            nullptr,
            0);

    pw_thread_loop_unlock(pLoop);

    if (res < 0) {
        const char* error = spa_strerror(res);
        qWarning() << "Error opening stream:" << error;
        m_lastError = QString::fromUtf8(error);
        return SoundDeviceStatus::Error;
    } else {
        qDebug() << "Opened Pipewire stream successfully... starting";
    }

    // update these in info callbacks
    if (m_isClkRef) {
        // Update the samplerate and latency ControlObjects, which allow the
        // waveform view to properly correct for the latency.
        ControlObject::set(
                ConfigKey(kAppGroup, QStringLiteral("output_latency_ms")),
                bufferMSec);
        ControlObject::set(ConfigKey(kAppGroup, QStringLiteral("samplerate")), m_sampleRate);
        m_invalidTimeInfoCount = 0;
        m_clkRefTimer.start();
    }

    return SoundDeviceStatus::Ok;
}

bool SoundDevicePipewire::isOpen() const {
    return m_pFilter != nullptr;
}

SoundDeviceStatus SoundDevicePipewire::close() {
    if (!isOpen()) {
        return SoundDeviceStatus::Ok;
    }

    m_closing = true;
    pw_thread_loop* threadLoop = m_pEnumerator->getThreadLoop();
    pw_thread_loop_lock(threadLoop);
    spa_hook_remove(&m_listener);
    pw_filter_disconnect(m_pFilter);
    pw_filter_destroy(m_pFilter);
    m_pFilter = nullptr;
    pw_thread_loop_unlock(threadLoop);

    m_inPorts.clear();
    m_outPorts.clear();
    m_bSetThreadPriority = false;

    return SoundDeviceStatus::Ok;
}

void SoundDevicePipewire::callback(const spa_io_position* pos) {
    Trace trace("SoundDevicePipewire::callbackProcess %1", m_deviceId.debugName());

    const spa_io_clock& clock = pos->clock;

    uint64_t framesPerBuffer = clock.duration;

    if (clock.flags & SPA_IO_CLOCK_FLAG_XRUN_RECOVER) {
        m_pSoundManager->underflowHappened(1);
        // qDebug() << "callbackProcess read:" << "Underflow";
    }

    for (auto& import : m_inPorts) {
        int writeAvailable = import.fifo->writeAvailable();

        if (writeAvailable > 0) {
            uint64_t toWrite = std::min<uint64_t>(writeAvailable, framesPerBuffer);
            void* buffer = pw_filter_get_dsp_buffer(import.fifo, toWrite);
            if (buffer == nullptr) {
                continue;
            }
            import.fifo->write(static_cast<const float*>(buffer), toWrite);

            if (toWrite < framesPerBuffer) {
                m_pSoundManager->underflowHappened(2);
                // qDebug() << "callbackProcess write:" << "Overflow";
            }
        } else {
            // Buffer full
            m_pSoundManager->underflowHappened(3);
            // qDebug() << "callbackProcess write:" << "Buffer full";
        }
    }

    for (auto& outPort : m_outPorts) {
        int readAvailable = outPort.fifo->readAvailable();

        if (readAvailable > 0) {
            uint64_t toWrite = std::min<uint64_t>(readAvailable, framesPerBuffer);
            void* buffer = pw_filter_get_dsp_buffer(outPort.fifo, toWrite);
            if (buffer == nullptr) {
                continue;
            }

            outPort.fifo->write(static_cast<const float*>(buffer), toWrite);

            if (toWrite < framesPerBuffer) {
                m_pSoundManager->underflowHappened(4);
                // qDebug() << "callbackProcess write:" << "Underflow";
            }
        } else {
            // Buffer full
            void* buffer = pw_filter_get_dsp_buffer(outPort.fifo, framesPerBuffer);
            if (buffer == nullptr) {
                continue;
            }

            SampleUtil::clear(static_cast<float*>(buffer), framesPerBuffer);
            m_pSoundManager->underflowHappened(5);
            // qDebug() << "callbackProcess write:" << "Buffer empty";
        }
    }
}

void SoundDevicePipewire::callbackDrift(const spa_io_position* pos) {
    Trace trace("SoundDevicePipewire::callbackProcessDrift %1",
            m_deviceId.debugName());

    const spa_io_clock& clock = pos->clock;
    int framesPerBuffer = clock.duration;

    if (clock.flags & SPA_IO_CLOCK_FLAG_XRUN_RECOVER) {
        m_pSoundManager->underflowHappened(7);
    }

    for (auto& import : m_inPorts) {
        auto& fifo = import.fifo;
        int readAvailable = fifo->readAvailable();
        int writeAvailable = fifo->writeAvailable();
        float* buffer = static_cast<float*>(pw_filter_get_dsp_buffer(import.fifo, framesPerBuffer));
        if (buffer == nullptr) {
            continue;
        }

        if (readAvailable < framesPerBuffer * kDriftReserve) {
            // risk of an underflow, duplicate one frame
            fifo->write(buffer, framesPerBuffer);
            if (m_inputDrift) {
                // Do not compensate the first delay, because it is likely a jitter
                // corrected in the next cycle
                // Duplicate one frame
                fifo->write(&buffer[framesPerBuffer - 1], 1);
                // qDebug() << "callbackProcessDrift write:" <<
                // (float)readAvailable / inChunkSize << "Skip";
            } else {
                m_inputDrift = true;
            }
        } else if (readAvailable == framesPerBuffer * kDriftReserve) {
            // Everything Ok
            fifo->write(buffer, framesPerBuffer);
            m_inputDrift = false;
            // qDebug() << "callbackProcess write:" << (float) readAvailable /
            // inChunkSize << "Normal";
        } else if (writeAvailable >= framesPerBuffer) {
            // Risk of overflow, skip one frame
            if (m_inputDrift) {
                fifo->write(buffer, framesPerBuffer - 1);
                // qDebug() << "callbackProcessDrift write:" <<
                // (float)readAvailable / inChunkSize << "Skip";
            } else {
                fifo->write(buffer, framesPerBuffer);
                m_inputDrift = true;
                // qDebug() << "callbackProcessDrift write:" <<
                // (float)readAvailable / inChunkSize << "Jitter Skip";
            }
        } else if (writeAvailable) {
            // Fifo Overflow
            fifo->write(buffer, writeAvailable);
            m_pSoundManager->underflowHappened(8);
            // qDebug() << "callbackProcessDrift write:" << (float)
            // readAvailable / inChunkSize << "Overflow";
        } else {
            // Buffer full
            m_pSoundManager->underflowHappened(9);
            // qDebug() << "callbackProcessDrift write:" << (float)
            // readAvailable / inChunkSize << "Buffer full";
        }
    }
    for (auto& outPort : m_outPorts) {
        auto& fifo = outPort.fifo;
        int readAvailable = fifo->readAvailable();
        float* buffer = static_cast<float*>(
                pw_filter_get_dsp_buffer(outPort.fifo, framesPerBuffer));
        if (buffer == nullptr) {
            continue;
        }

        if (readAvailable > framesPerBuffer * (kDriftReserve + 1)) {
            fifo->read(buffer, framesPerBuffer);
            if (m_outputDrift) {
                // Risk of overflow, skip one frame
                fifo->releaseReadRegions(1);
                // qDebug() << "callbackProcessDrift read:" <<
                // (float)readAvailable / outChunkSize << "Skip";
            } else {
                m_outputDrift = true;
                // qDebug() << "callbackProcessDrift read:" <<
                // (float)readAvailable / outChunkSize << "Jitter Skip";
            }
        } else if (readAvailable == framesPerBuffer * (kDriftReserve + 1)) {
            fifo->read(buffer, framesPerBuffer);
            m_outputDrift = false;
            // qDebug() << "callbackProcessDrift read:" << (float)readAvailable
            // / outChunkSize << "Normal";
        } else if (readAvailable >= framesPerBuffer) {
            if (m_outputDrift) {
                // Risk of underflow, duplicate one frame
                fifo->read(buffer, framesPerBuffer - 1);
                SampleUtil::copy(
                        &buffer[framesPerBuffer - 1],
                        &buffer[framesPerBuffer - 2],
                        1);
                // qDebug() << "callbackProcessDrift read:" <<
                // (float)readAvailable / outChunkSize << "Save";
            } else {
                fifo->read(buffer, framesPerBuffer);
                m_outputDrift = true;
                // qDebug() << "callbackProcessDrift read:" <<
                // (float)readAvailable / outChunkSize << "Jitter Save";
            }
        } else if (readAvailable) {
            fifo->read(buffer, readAvailable);
            // underflow
            SampleUtil::clear(&buffer[readAvailable], framesPerBuffer - readAvailable);
            m_pSoundManager->underflowHappened(10);
            // qDebug() << "callbackProcessDrift read:" << (float)readAvailable
            // / outChunkSize << "Underflow";
        } else {
            // underflow
            SampleUtil::clear(buffer, framesPerBuffer);
            m_pSoundManager->underflowHappened(11);
            // qDebug() << "callbackProcess read:" << (float)readAvailable /
            // outChunkSize << "Buffer empty";
        }
    }
}

void SoundDevicePipewire::writeInput(
        const float* input, int channel, int framesPerBuffer, int offset = 0) {
    for (auto i = m_audioInputs.constBegin(), e = m_audioInputs.constEnd(); i != e; ++i) {
        const AudioInputBuffer& in = *i;
        ChannelGroup chanGroup = in.getChannelGroup();
        const int iChannelCount = chanGroup.getChannelCount();
        const int iChannelBase = chanGroup.getChannelBase();
        const int iChannelEnd = iChannelCount + iChannelBase;

        if (channel < iChannelBase || channel > iChannelEnd) {
            continue;
        }

        CSAMPLE* pInputBuffer = &in.getBuffer()[offset];

        if (iChannelCount == 1) {
            for (int i = 0; i < framesPerBuffer; i++) {
                pInputBuffer[i] = input[i];
                pInputBuffer[i + 1] = input[i];
            }
        } else {
            for (int i = 0; i < framesPerBuffer; i++) {
                pInputBuffer[i * iChannelCount + channel] = input[i];
            }
        }
    }
}

void SoundDevicePipewire::writeOutput(
        float* output, int channel, int framesPerBuffer, int offset = 0) {
    for (auto i = m_audioOutputs.constBegin(), e = m_audioOutputs.constEnd(); i != e; ++i) {
        const AudioOutputBuffer& in = *i;
        ChannelGroup chanGroup = in.getChannelGroup();
        const int iChannelCount = chanGroup.getChannelCount();
        const int iChannelBase = chanGroup.getChannelBase();
        const int iChannelEnd = iChannelCount + iChannelBase;

        if (channel < iChannelBase || channel > iChannelEnd) {
            continue;
        }

        const CSAMPLE* pOutputBuffer = &in.getBuffer()[offset];

        if (iChannelCount == 1) {
            for (int i = 0; i < framesPerBuffer; i++) {
                output[i] = pOutputBuffer[i * 2];
            }
        } else {
            for (int i = 0; i < framesPerBuffer; i++) {
                output[i] = pOutputBuffer[i * iChannelCount + channel];
            }
        }
    }
}

void SoundDevicePipewire::callbackClkRef(const spa_io_position* pos) {
    // This must be the very first call, else timeInfo becomes invalid
    updateCallbackEntryToDacTime(pos);

    Trace trace("SoundDevicePipewire::callbackProcessClkRef %1",
            m_deviceId.debugName());

    // copied from SoundDevicePortAudio, extract it to common utility?

    if (!m_bSetThreadPriority) {
#ifdef __LINUX__
        // Verify if we are a thread with "real-time" policy.
        // The audio thread on Linux should be set to SCHED_FIFO with a priority
        // that's somewhere between 60 and 90 depending on the allowed priority
        // ranges (some USB devices by default get assigned a priority in the
        // 50s with some system configs).
        if ((sched_getscheduler(0) & SCHED_FIFO) == 0) {
            qWarning() << "Engine thread not scheduled with the real-time policy SCHED_FIFO";
        }
#else
        // Turn on TimeCritical priority for the callback thread.
        // If we are running in Linux this will have no effect. Either the thread is
        // already set up correctly because of the audio server, or it's still set to
        // the SCHED_OTHER policy in which case the call also wouldn't do anything.
        QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
#endif
        m_bSetThreadPriority = true;

        // This disables the denormals calculations, to avoid a
        // performance penalty of ~20
        // https://github.com/mixxxdj/mixxx/issues/7747

        // On Emscripten (WebAssembly) denormals-as-zero/flush-as-zero are
        // neither supported nor configurable. This may lead to degraded
        // performance compared to other platforms and may be addressed in the
        // future if/when WebAssembly adds support for DAZ/FTZ. For further
        // discussion and links see https://github.com/mixxxdj/mixxx/pull/12917

#if defined(__SSE__) && !defined(__EMSCRIPTEN__)
        if (!_MM_GET_DENORMALS_ZERO_MODE()) {
            qDebug() << "SSE: Enabling denormals to zero mode";
            _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
        } else {
            qDebug() << "SSE: Denormals to zero mode already enabled";
        }

        if (!_MM_GET_FLUSH_ZERO_MODE()) {
            qDebug() << "SSE: Enabling flush to zero mode";
            _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
        } else {
            qDebug() << "SSE: Flush to zero mode already enabled";
        }
#else
#if defined(__i386__) || defined(__i486__) || defined(__i586__) || \
        defined(__i686__) || defined(__x86_64__) || defined(_M_I86)
        qWarning() << "No SSE: No denormals to zero mode available. EQs and "
                      "effects may suffer high CPU load";
#endif
#endif

#if defined(__aarch64__)
        // Flush-to-zero on aarch64 is controlled by the Floating-point Control Register
        // Load the register into our variable.
        int64_t savedFPCR;
        asm volatile("mrs %[savedFPCR], FPCR"
                : [savedFPCR] "=r"(savedFPCR));

        qDebug() << "aarch64 FPCR: setting bit 24 to 1 to enable Flush-to-zero";
        // Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
        asm volatile("msr FPCR, %[src]"
                :
                : [src] "r"(savedFPCR | (1 << 24)));
#endif

        // verify if flush to zero or denormals to zero works
        // test passes if one of the two flag is set.
        volatile double doubleMin = DBL_MIN; // the smallest normalized double
#if __has_include(<valgrind/valgrind.h>)
        if (RUNNING_ON_VALGRIND) {
            qDebug() << "Skipping denormals to zero check: running under Valgrind";
        } else
#endif
            VERIFY_OR_DEBUG_ASSERT(doubleMin / 2 == 0.0) {
                qWarning() << "Denormals to zero mode is not working. EQs and "
                              "effects may suffer high CPU load";
            }
        else {
            qDebug() << "Denormals to zero mode is working";
        }
    }

#ifdef __SSE__
#ifdef __WINDOWS__
    // We need to refresh the denormals flags every callback since some
    // driver + API combinations will reset them (known: DirectSound + Realtec)
    // Fixes issue #8220
    // (Both calls are very fast)
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#endif
#endif

    if (pos->clock.flags & SPA_IO_CLOCK_FLAG_XRUN_RECOVER) {
        m_pSoundManager->underflowHappened(6);
    }
    const uint64_t framesPerBuffer = pos->clock.duration;
    m_pSoundManager->processUnderflowHappened(framesPerBuffer);

    // Note: Input is processed first so that any ControlObject changes made in
    //       response to input are processed as soon as possible (that is, when
    //       m_pSoundManager->requestBuffer() is called below.)

    // Send audio from the soundcard's input off to the SoundManager...
    for (uint i = 0; i < m_inPorts.size(); i++) {
        const float* input = static_cast<const float*>(
                pw_filter_get_dsp_buffer(m_inPorts[i].fifo, framesPerBuffer));
        if (input == nullptr) {
            continue;
        }
        writeInput(input, i, framesPerBuffer);
    }

    m_pSoundManager->pushInputBuffers(m_audioInputs, framesPerBuffer);

    m_pSoundManager->readProcess(framesPerBuffer);

    {
        ScopedTimer t(QStringLiteral("SoundDevicePipewire::callbackProcess prepare %1"),
                m_deviceId.debugName());
        m_pSoundManager->onDeviceOutputCallback(framesPerBuffer);
    }

    for (uint i = 0; i < m_outPorts.size(); i++) {
        float* output = static_cast<float*>(
                pw_filter_get_dsp_buffer(m_outPorts[i].fifo, framesPerBuffer));
        if (output == nullptr) {
            continue;
        }

        SampleUtil::clear(output, framesPerBuffer);
        writeOutput(output, i, framesPerBuffer);
    }

    m_pSoundManager->writeProcess(framesPerBuffer);
    updateAudioLatencyUsage(framesPerBuffer);
}

void SoundDevicePipewire::updateCallbackEntryToDacTime(const spa_io_position* timeInfo) {
    double timeSinceLastCbSecs = m_clkRefTimer.restart().toDoubleSeconds();
    const spa_io_clock& clock = timeInfo->clock;

    // read comments in SoundDevicePortAudio function

    double callbackEntrytoDacSecs = clock.delay / clock.rate.denom;
    double bufferSizeSec = clock.duration / m_sampleRate.toDouble();

    double diff = (timeSinceLastCbSecs + callbackEntrytoDacSecs) -
            (m_lastCallbackEntrytoDacSecs + bufferSizeSec);

    if (callbackEntrytoDacSecs <= 0 ||
            (timeSinceLastCbSecs < bufferSizeSec * 2 &&
                    fabs(diff) / bufferSizeSec > 0.1)) {
        // Fall back to CPU timing:
        // If timeSinceLastCbSecs from a CPU timer is reasonable (no underflow),
        // the callbackEntrytoDacSecs time is not in the past
        // and we have more than 10 % difference to the timing provided by Portaudio
        // we do not trust the Portaudio timing.
        // (A difference up to ~ 5 % is normal)

        m_invalidTimeInfoCount++;

        if (m_invalidTimeInfoCount == m_invalidTimeInfoWarningCount) {
            if (CmdlineArgs::Instance().getDeveloper()) {
                qWarning() << "SoundDevicePipewire: Audio API provides invalid time stamps,"
                           << "syncing waveforms with a CPU Timer"
                           << "DacTime:" << clock.delay + clock.nsec
                           << "EntrytoDac:" << callbackEntrytoDacSecs
                           << "TimeSinceLastCb:" << timeSinceLastCbSecs
                           << "diff:" << diff;
            }
        }

        callbackEntrytoDacSecs =
                (m_lastCallbackEntrytoDacSecs + bufferSizeSec) -
                timeSinceLastCbSecs;
        // clamp values to avoid a big offset due to clock drift.
        callbackEntrytoDacSecs = math_clamp(callbackEntrytoDacSecs, 0.0, bufferSizeSec * 2);
    }

    VisualPlayPosition::setCallbackEntryToDacSecs(callbackEntrytoDacSecs, m_clkRefTimer);
    m_lastCallbackEntrytoDacSecs = callbackEntrytoDacSecs;

    // qDebug() << callbackEntrytoDacSecs << timeSinceLastCbSecs;
}

void SoundDevicePipewire::updateAudioLatencyUsage(
        const SINT framesPerBuffer) {
    m_framesSinceAudioLatencyUsageUpdate += framesPerBuffer;
    if (m_framesSinceAudioLatencyUsageUpdate > (m_sampleRate.toDouble() / kCpuUsageUpdateRate)) {
        double secInAudioCb = m_timeInAudioCallback.toDoubleSeconds();
        m_audioLatencyUsage.set(
                secInAudioCb / (m_framesSinceAudioLatencyUsageUpdate / m_sampleRate.toDouble()));
        m_timeInAudioCallback = mixxx::Duration::fromSeconds(0);
        m_framesSinceAudioLatencyUsageUpdate = 0;
        // qDebug() << m_audioLatencyUsage
        //          << m_audioLatencyUsage->get();
    }
    // measure time in Audio callback at the very last
    m_timeInAudioCallback += m_clkRefTimer.elapsed();
}

void SoundDevicePipewire::callback(void* data, spa_io_position* pos) {
    SoundDevicePipewire* device = static_cast<SoundDevicePipewire*>(data);
    if (!device->m_closing) {
        ((SoundDevicePipewire*)data)->callback(pos);
    }
}

void SoundDevicePipewire::callbackDrift(void* data, spa_io_position* pos) {
    SoundDevicePipewire* device = static_cast<SoundDevicePipewire*>(data);
    if (!device->m_closing) {
        ((SoundDevicePipewire*)data)->callbackDrift(pos);
    }
}

void SoundDevicePipewire::callbackClkRef(void* data, spa_io_position* pos) {
    SoundDevicePipewire* device = static_cast<SoundDevicePipewire*>(data);
    if (!device->m_closing) {
        ((SoundDevicePipewire*)data)->callbackClkRef(pos);
    }
}

void SoundDevicePipewire::readProcess(SINT framesPerBuffer) {
    if (m_isClkRef) {
        return; // clkRef devices still have fifo of 0 size
    }

    for (auto& port : m_inPorts) {
        auto& fifo = port.fifo;
        if (m_syncBuffers == 0) { // "Experimental (no delay)"
            int readAvailable = fifo->readAvailable();
            if (readAvailable == 0) {
                // Initial call or underflow at last call
                // Init half of the buffer with silence
                CSAMPLE* dataPtr1;
                ring_buffer_size_t size1;
                CSAMPLE* dataPtr2;
                ring_buffer_size_t size2;
                (void)fifo->aquireWriteRegions(framesPerBuffer,
                        &dataPtr1,
                        &size1,
                        &dataPtr2,
                        &size2);
                // Fetch fresh samples and write to the the input buffer
                SampleUtil::clear(dataPtr1, size1);
                if (size2 > 0) {
                    SampleUtil::clear(dataPtr2, size2);
                }
                fifo->releaseWriteRegions(framesPerBuffer);
            }
            int writeAvailable = fifo->writeAvailable();
            int copyCount = std::min<int>(writeAvailable, framesPerBuffer);
            // qDebug() << "readProcess()" << (float)writeAvailable /
            // inChunkSize << (float)readAvailable / inChunkSize;
            if (copyCount > 0) {
                CSAMPLE* dataPtr1;
                ring_buffer_size_t size1;
                CSAMPLE* dataPtr2;
                ring_buffer_size_t size2;
                (void)fifo->aquireWriteRegions(copyCount,
                        &dataPtr1,
                        &size1,
                        &dataPtr2,
                        &size2);
                CSAMPLE* lastFrame = &dataPtr1[size1 - 1];
                // Fetch fresh samples and write to the the input buffer
                float* buffer = static_cast<float*>(pw_filter_get_dsp_buffer(port.fifo, copyCount));
                if (buffer == nullptr) {
                    continue;
                }
                SampleUtil::copy(dataPtr1, buffer, size1);

                if (size2 > 0) {
                    SampleUtil::copy(dataPtr1, &buffer[size1], size2);
                    lastFrame = &dataPtr2[size2 - 1];
                }
                fifo->releaseWriteRegions(copyCount);

                if (readAvailable > writeAvailable + framesPerBuffer / 2) {
                    // we are not able to consume enough frames
                    if (m_inputDrift) {
                        // Skip one frame
                        // qDebug() << "SoundDevicePipewire::readProcess() skip
                        // one frame"
                        //        << (float)writeAvailable / inChunkSize <<
                        //        (float)readAvailable / inChunkSize;
                        //
                        // pw_filter trivially discards sample if not consumed
                        // in one cycle
                    } else {
                        m_inputDrift = true;
                    }
                } else if (readAvailable < framesPerBuffer / 2 ||
                        fifo->readAvailable() < framesPerBuffer * 1.5) {
                    // We should read at least a half inChunkSize
                    // and our m_iputFifo should now hold a half chunk extra
                    if (m_inputDrift) {
                        // duplicate one frame
                        // qDebug() << "SoundDevicePipewire::readProcess()
                        // duplicate one frame"
                        //        << (float)writeAvailable / inChunkSize <<
                        //        (float)readAvailable / inChunkSize;
                        (void)fifo->aquireWriteRegions(
                                1, &dataPtr1, &size1, &dataPtr2, &size2);
                        if (size1) {
                            SampleUtil::copy(dataPtr1, lastFrame, size1);
                            fifo->releaseWriteRegions(size1);
                        }
                    } else {
                        m_inputDrift = true;
                    }
                } else {
                    m_inputDrift = false;
                }
            }
        }
    }
    for (uint channel = 0; channel < m_inPorts.size(); channel++) {
        auto& port = m_inPorts[channel];
        auto& fifo = port.fifo;
        int inChunkSize = framesPerBuffer;
        if (m_syncBuffers == 0) { // "Experimental (no delay)"
            if (fifo->readAvailable() == 0) {
                // Initial call or underflow at last call
                // Init half of the buffer with silence
                CSAMPLE* dataPtr1;
                ring_buffer_size_t size1;
                CSAMPLE* dataPtr2;
                ring_buffer_size_t size2;
                (void)fifo->aquireWriteRegions(inChunkSize,
                        &dataPtr1,
                        &size1,
                        &dataPtr2,
                        &size2);
                // Fetch fresh samples and write to the the input buffer
                SampleUtil::clear(dataPtr1, size1);
                if (size2 > 0) {
                    SampleUtil::clear(dataPtr2, size2);
                }
                fifo->releaseWriteRegions(inChunkSize);
            }

            // Polling mode
            // signed int readAvailable = Pa_GetStreamReadAvailable(pStream) *
            // m_inputParams.channelCount;
            int readAvailable = framesPerBuffer;
            int writeAvailable = fifo->writeAvailable();
            int copyCount = qMin(writeAvailable, readAvailable);
            // qDebug() << "readProcess()" << (float)writeAvailable /
            // inChunkSize << (float)readAvailable / inChunkSize;
            if (copyCount > 0) {
                CSAMPLE* dataPtr1;
                ring_buffer_size_t size1;
                CSAMPLE* dataPtr2;
                ring_buffer_size_t size2;
                (void)fifo->aquireWriteRegions(copyCount,
                        &dataPtr1,
                        &size1,
                        &dataPtr2,
                        &size2);
                // Fetch fresh samples and write to the the input buffer
                float* buffer = static_cast<float*>(
                        pw_filter_get_dsp_buffer(port.fifo, framesPerBuffer));
                if (buffer == nullptr) {
                    continue;
                }
                SampleUtil::copy(dataPtr1, buffer, size1);
                CSAMPLE* lastFrame = &dataPtr1[size1 - 1];
                if (size2 > 0) {
                    SampleUtil::copy(dataPtr2, &buffer[size1], size2);
                    lastFrame = &dataPtr2[size2 - 1];
                }
                fifo->releaseWriteRegions(copyCount);

                // not skipping sample in case of input drift
                // since pw_filter does not buffer samples, so
                // any samples not used are automatically skipped
                if (readAvailable > writeAvailable + inChunkSize / 2) {
                    m_inputDrift = true;
                } else if (readAvailable < inChunkSize / 2 ||
                        fifo->readAvailable() < inChunkSize * 1.5) {
                    // We should read at least a half inChunkSize
                    // and our m_iputFifo should now hold a half chunk extra
                    if (m_inputDrift) {
                        // duplicate one frame
                        // qDebug() << "SoundDevicePipewire::readProcess()
                        // duplicate one frame"
                        //        << (float)writeAvailable / inChunkSize <<
                        //        (float)readAvailable / inChunkSize;
                        (void)fifo->aquireWriteRegions(
                                1, &dataPtr1, &size1, &dataPtr2, &size2);
                        if (size1) {
                            SampleUtil::copy(dataPtr1, lastFrame, size1);
                            fifo->releaseWriteRegions(size1);
                        }
                    } else {
                        m_inputDrift = true;
                    }
                } else {
                    m_inputDrift = false;
                }
            }
        }

        int readAvailable = fifo->readAvailable();
        int readCount = inChunkSize;
        if (inChunkSize > readAvailable) {
            readCount = readAvailable;
            m_pSoundManager->underflowHappened(15);
            // qDebug() << "readProcess()" << (float)readAvailable / inChunkSize << "underflow";
        }
        // qDebug() << "readProcess()" << (float)readAvailable / inChunkSize;
        if (readCount) {
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;
            // We use size1 and size2, so we can ignore the return value
            (void)fifo->aquireReadRegions(readCount, &dataPtr1, &size1, &dataPtr2, &size2);
            // Fetch fresh samples and write to the the output buffer
            writeInput(dataPtr1, channel, size1);
            if (size2 > 0) {
                writeInput(dataPtr2, channel, size2, size1);
            }
            fifo->releaseReadRegions(readCount);
        }
        if (readCount < inChunkSize) {
            // Fill remaining buffers with zeros
            clearInputBuffer(inChunkSize - readCount, readCount);
        }

        m_pSoundManager->pushInputBuffers(m_audioInputs, framesPerBuffer);
    }
}
void SoundDevicePipewire::writeProcess(SINT framesPerBuffer) {
    if (m_isClkRef) {
        return; // clkRef devices still have fifo of 0 size
    }

    for (uint channel = 0; channel < m_outPorts.size(); channel++) {
        auto& port = m_outPorts[channel];
        auto* fifo = port.fifo;

        int outChunkSize = framesPerBuffer;
        int writeAvailable = fifo->writeAvailable();
        int writeCount = outChunkSize;
        if (outChunkSize > writeAvailable) {
            writeCount = writeAvailable;
            m_pSoundManager->underflowHappened(16);
            // qDebug() << "writeProcess():" << (float) writeAvailable / outChunkSize << "Overflow";
        }
        if (writeCount > 0) {
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;
            // We use size1 and size2, so we can ignore the return value
            (void)fifo->aquireWriteRegions(writeCount, &dataPtr1, &size1, &dataPtr2, &size2);
            // Fetch fresh samples and write to the the output buffer
            writeOutput(dataPtr1, channel, size1);
            if (size2 > 0) {
                writeOutput(dataPtr2, channel, size2, size1);
            }
            fifo->releaseWriteRegions(writeCount);
        }

        if (m_syncBuffers == 0) { // "Experimental (no delay)"
            // Polling
            signed int writeAvailable = framesPerBuffer;
            int readAvailable = fifo->readAvailable();
            int copyCount = qMin(readAvailable, writeAvailable);
            // qDebug() << "SoundDevicePipewire::writeProcess()" <<
            // (float)readAvailable / outChunkSize << (float)writeAvailable /
            // outChunkSize;
            if (copyCount > 0) {
                float* buffer = static_cast<float*>(
                        pw_filter_get_dsp_buffer(port.fifo, framesPerBuffer));
                if (buffer == nullptr) {
                    continue;
                }
                uint offset = 0;
                CSAMPLE* dataPtr1;
                ring_buffer_size_t size1;
                CSAMPLE* dataPtr2;
                ring_buffer_size_t size2;
                fifo->aquireReadRegions(copyCount,
                        &dataPtr1,
                        &size1,
                        &dataPtr2,
                        &size2);
                if (writeAvailable >= outChunkSize * 2) {
                    // Underflow (2 is max for native ALSA devices)
                    // qDebug() << "SoundDevicePipewire::writeProcess() fill
                    // buffer" << (float)(writeAvailable - copyCount) /
                    // outChunkSize; fill buffer with duplicate of first sample
                    SampleUtil::copy(&buffer[offset], dataPtr1, writeAvailable - copyCount);
                    offset = writeAvailable - copyCount;
                    m_pSoundManager->underflowHappened(17);
                } else if (writeAvailable > readAvailable + outChunkSize / 2) {
                    // try to keep PAs buffer filled up to 0.5 chunks
                    if (m_outputDrift) {
                        // duplicate one frame
                        // qDebug() << "SoundDevicePipewire::writeProcess()
                        // duplicate one frame"
                        //        << (float)writeAvailable / outChunkSize <<
                        //        (float)readAvailable / outChunkSize;

                        // implement cross graph drift correction
                        SampleUtil::copy(&buffer[offset], dataPtr1, 1);
                        offset++;
                    } else {
                        // qDebug() << "SoundDevicePipewire::writeProcess() OK"
                        // << (float)writeAvailable / outChunkSize <<
                        // (float)readAvailable / outChunkSize;
                        m_outputDrift = true;
                    }
                } else if (writeAvailable < outChunkSize / 2 ||
                        readAvailable - copyCount > outChunkSize * 0.5) {
                    // We are not able to store at least the half of the new frames
                    // or we have a risk of an fifo overflow
                    if (m_outputDrift) {
                        // qDebug() << "SoundDevicePipewire::writeProcess() skip
                        // one frame"
                        //         << (float)writeAvailable / outChunkSize <<
                        //         (float)readAvailable / outChunkSize;
                        copyCount = qMin(readAvailable, copyCount + 1);
                    } else {
                        m_outputDrift = true;
                    }
                } else {
                    m_outputDrift = false;
                }
                SampleUtil::copy(&buffer[offset], dataPtr1, size1);
                if (size2 > 0) {
                    SampleUtil::copy(&buffer[offset], dataPtr2, size2);
                }
                fifo->releaseReadRegions(copyCount);
            }
        }
    }
}

FIFO<CSAMPLE>* SoundDevicePipewire::addFilterPort(spa_direction dir, uint8_t index) {
    std::string name = (dir == SPA_DIRECTION_OUTPUT ? "out:" : "in:") + std::to_string(index);
    void* data = pw_filter_add_port(m_pFilter,
            dir,
            PW_FILTER_PORT_FLAG_MAP_BUFFERS,
            m_fifoSize,
            pw_properties_new("mixxx.filter.target",
                    std::to_string(m_deviceId.deviceIndex).c_str(),
                    PW_KEY_FORMAT_DSP,
                    "32 bit float mono audio",
                    PW_KEY_PORT_NAME,
                    name.c_str(),
                    nullptr),
            nullptr,
            0);

    DEBUG_ASSERT(data != nullptr);

    return static_cast<FIFO<CSAMPLE>*>(data);
}

void SoundDevicePipewire::createLink(uint32_t outNodeId,
        uint32_t outPortId,
        uint32_t inNodeId,
        uint32_t inPortId) {
    spa_dict_item items[6];
    spa_dict props = SPA_DICT_INIT(items, 0);

    std::string strOutNode = std::to_string(outNodeId);
    std::string strOutPort = std::to_string(outPortId);
    std::string strInNode = std::to_string(inNodeId);
    std::string strInPort = std::to_string(inPortId);

    items[props.n_items++] = SPA_DICT_ITEM_INIT(PW_KEY_LINK_OUTPUT_NODE, strOutNode.c_str());
    items[props.n_items++] = SPA_DICT_ITEM_INIT(PW_KEY_LINK_OUTPUT_PORT, strOutPort.c_str());
    items[props.n_items++] = SPA_DICT_ITEM_INIT(PW_KEY_LINK_INPUT_NODE, strInNode.c_str());
    items[props.n_items++] = SPA_DICT_ITEM_INIT(PW_KEY_LINK_INPUT_PORT, strInPort.c_str());
    items[props.n_items++] = SPA_DICT_ITEM_INIT(PW_KEY_OBJECT_LINGER, "true");

    struct pw_proxy* proxy = (struct pw_proxy*)pw_core_create_object(m_pEnumerator->getCore(),
            "link-factory",
            PW_TYPE_INTERFACE_Link,
            PW_VERSION_LINK,
            &props,
            0);
    if (proxy) {
        pw_proxy_destroy(proxy);
    }
}

void SoundDevicePipewire::registerFilterPort(uint32_t id, const spa_dict* props) {
    const uint32_t filter_id = pw_filter_get_node_id(m_pFilter);
    const char* index_str = spa_dict_lookup(props, PW_KEY_PORT_ID);
    const char* direction = spa_dict_lookup(props, PW_KEY_PORT_DIRECTION);
    const int index = pw_properties_parse_int(index_str);

    if (std::strcmp(direction, "in") == 0) {
        m_inPorts[index].id = id;
        if (isOpen()) {
            createLink(m_deviceId.deviceIndex,
                    m_outDevicePorts[index].id,
                    filter_id,
                    m_inPorts[index].id);
        }
    } else {
        m_outPorts[index].id = id;
        if (isOpen()) {
            createLink(filter_id,
                    m_outPorts[index].id,
                    m_deviceId.deviceIndex,
                    m_inDevicePorts[index].id);
        }
    }
}

void SoundDevicePipewire::registerLink(uint32_t id, spa_direction dir, spa_dict const* props) {
    const char* port_str;
    int port_id;

    if (dir == SPA_DIRECTION_INPUT) {
        port_str = spa_dict_lookup(props, PW_KEY_LINK_OUTPUT_PORT);
        port_id = pw_properties_parse_int(port_str);
    } else {
        port_str = spa_dict_lookup(props, PW_KEY_LINK_INPUT_PORT);
        port_id = pw_properties_parse_int(port_str);
    }

    DevicePort* port = findPort(port_id);
    if (port) {
        port->links.emplace_back(id);
    }
}

void SoundDevicePipewire::unregisterLink(uint32_t id, uint32_t portIndex, spa_direction direction) {
    std::vector<DevicePort::Link>* links;

    if (direction == SPA_DIRECTION_INPUT) {
        links = &m_inDevicePorts[portIndex].links;
    } else {
        links = &m_outDevicePorts[portIndex].links;
    }

    for (auto it = links->begin(); it != links->end(); it++) {
        if (it->id == id) {
            links->erase(it);
            return;
        }
    }
}

SoundDevicePipewire::DevicePort* SoundDevicePipewire::findPort(uint32_t port_id) {
    for (auto& port : m_inDevicePorts) {
        if (port.id == port_id) {
            return &port;
        }
    }

    for (auto& port : m_outDevicePorts) {
        if (port.id == port_id) {
            return &port;
        }
    }

    return nullptr;
}

QString SoundDevicePipewire::getError() const {
    return m_lastError;
}
