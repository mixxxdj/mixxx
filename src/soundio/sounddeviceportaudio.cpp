#include "soundio/sounddeviceportaudio.h"

#include <float.h>

#include <QRegularExpression>
#include <QThread>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "soundio/sounddevice.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"
#include "util/denormalsarezero.h"
#include "util/fifo.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/timer.h"
#include "util/trace.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "waveform/visualplayposition.h"

#ifdef __LINUX__
// for PaAlsa_EnableRealtimeScheduling
#include <pa_linux_alsa.h>
#endif

Q_LOGGING_CATEGORY(mixxxLogDevicePortAudio, MIXXX_LOGGING_CATEGORY_DEVICE_PORTAUDIO)

// Enable verbose trace logs temporarily for debugging.
//
// DISABLE TRACE LOGGING BEFORE COMITTING ANY CHANGES!
// Logging in real-time code is strictly forbidden.
#define ENABLE_TRACE_LOG false

namespace {

// Buffer for drift correction 1 full, 1 for r/w, 1 empty
constexpr int kDriftReserve = 1;

// Buffer for drift correction 1 full, 1 for r/w, 1 empty
constexpr int kFifoSize = 2 * kDriftReserve + 1;

constexpr int kCpuUsageUpdateRate = 30; // in 1/s, fits to display frame rate

// We warn only at invalid timing 3, since the first two
// callbacks can be always wrong due to a setup/open jitter
constexpr int kInvalidTimeInfoWarningCount = 3;

constexpr double kDefaultSampleRate = 44100.0;

/** Dynamically resolved function which allows us to enable a realtime-priority callback
    thread from ALSA/PortAudio. This must be dynamically resolved because PortAudio can't
    tell us if ALSA is compiled into it or not. */
typedef int (*EnableAlsaRT)(PaStream* s, int enable);

int paV19Callback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *soundDevice) {
    return ((SoundDevicePortAudio*) soundDevice)->callbackProcess(
            (SINT) framesPerBuffer, (CSAMPLE*) outputBuffer,
            (const CSAMPLE*) inputBuffer, timeInfo, statusFlags);
}

int paV19CallbackDrift(const void *inputBuffer, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags statusFlags,
                       void *soundDevice) {
    return ((SoundDevicePortAudio*) soundDevice)->callbackProcessDrift(
            (SINT) framesPerBuffer, (CSAMPLE*) outputBuffer,
            (const CSAMPLE*) inputBuffer, timeInfo, statusFlags);
}

int paV19CallbackClkRef(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *soundDevice) {
    return ((SoundDevicePortAudio*) soundDevice)->callbackProcessClkRef(
            (SINT) framesPerBuffer, (CSAMPLE*) outputBuffer,
            (const CSAMPLE*) inputBuffer, timeInfo, statusFlags);
}

const QRegularExpression kAlsaHwDeviceRegex(
        QStringLiteral("(.*) \\((plug)?(hw:(\\d)+(,(\\d)+))?\\)"));

} // anonymous namespace

SoundDevicePortAudio::SoundDevicePortAudio(UserSettingsPointer config,
                                           SoundManager* sm,
                                           const PaDeviceInfo* deviceInfo,
                                           unsigned int devIndex,
                                           QHash<PaHostApiIndex, PaHostApiTypeId> apiIndexToTypeId)
        : SoundDevice(config, sm),
          m_deviceInfo(deviceInfo),
          m_outputDrift(false),
          m_inputDrift(false),
          m_bSetThreadPriority(false),
          m_framesSinceAudioLatencyUsageUpdate(0),
          m_syncBuffers(2),
          m_invalidTimeInfoCount(0),
          m_lastCallbackEntrytoDacSecs(0) {
    // Setting parent class members:
    m_hostAPI = Pa_GetHostApiInfo(deviceInfo->hostApi)->name;
    m_dSampleRate = deviceInfo->defaultSampleRate;
    if (apiIndexToTypeId.value(deviceInfo->hostApi) == paALSA) {
        // PortAudio gives the device name including the ALSA hw device. The
        // ALSA hw device is an only somewhat reliable identifier; it may change
        // when an audio interface is unplugged or Linux is restarted. Separating
        // the name from the hw device allows for making the use of both pieces
        // of information in SoundManagerConfig::readFromDisk to minimize how
        // often users need to reconfigure their sound hardware.
        QRegularExpressionMatch match = kAlsaHwDeviceRegex.match(deviceInfo->name);
        if (match.hasMatch()) {
            m_deviceId.name = match.captured(1);
            m_deviceId.alsaHwDevice = match.captured(3);
        } else {
            // Special ALSA devices like "default" and "pulse" do not match the regex
            m_deviceId.name = deviceInfo->name;
        }
    } else {
        m_deviceId.name = deviceInfo->name;
    }
    m_deviceId.portAudioIndex = devIndex;
    m_strDisplayName = QString::fromLocal8Bit(deviceInfo->name);
    m_iNumInputChannels = m_deviceInfo->maxInputChannels;
    m_iNumOutputChannels = m_deviceInfo->maxOutputChannels;

    m_pMasterAudioLatencyUsage = new ControlProxy("[Master]",
            "audio_latency_usage");

    m_inputParams.device = 0;
    m_inputParams.channelCount = 0;
    m_inputParams.sampleFormat = 0;
    m_inputParams.suggestedLatency = 0.0;
    m_inputParams.hostApiSpecificStreamInfo = nullptr;

    m_outputParams.device = 0;
    m_outputParams.channelCount = 0;
    m_outputParams.sampleFormat = 0;
    m_outputParams.suggestedLatency = 0.0;
    m_outputParams.hostApiSpecificStreamInfo = nullptr;
}

SoundDevicePortAudio::~SoundDevicePortAudio() {
    delete m_pMasterAudioLatencyUsage;
}

unsigned int SoundDevicePortAudio::getDefaultSampleRate() const {
    return static_cast<unsigned int>(
            m_deviceInfo ? m_deviceInfo->defaultSampleRate : kDefaultSampleRate);
}

SoundDeviceError SoundDevicePortAudio::open(bool isClkRefDevice, int syncBuffers) {
    qCInfo(mixxxLogDevicePortAudio)
            << "Opening device"
            << m_deviceId.name;
    PaError err;

    if (m_audioOutputs.empty() && m_audioInputs.empty()) {
        m_lastError = QStringLiteral(
                "No inputs or outputs in SDPA::open() "
                "(THIS IS A BUG, this should be filtered by SM::setupDevices)");
        return SOUNDDEVICE_ERROR_ERR;
    }

    memset(&m_outputParams, 0, sizeof(m_outputParams));
    memset(&m_inputParams, 0, sizeof(m_inputParams));
    PaStreamParameters* pOutputParams = &m_outputParams;
    PaStreamParameters* pInputParams = &m_inputParams;

    // Look at how many audio outputs we have,
    // so we can figure out how many output channels we need to open.
    if (m_audioOutputs.empty()) {
        m_outputParams.channelCount = 0;
        pOutputParams = nullptr;
    } else {
        foreach (AudioOutput out, m_audioOutputs) {
            ChannelGroup channelGroup = out.getChannelGroup();
            int highChannel = channelGroup.getChannelBase()
                    + channelGroup.getChannelCount();
            if (m_outputParams.channelCount <= highChannel) {
                m_outputParams.channelCount = highChannel;
            }
        }
    }

    // Look at how many audio inputs we have,
    // so we can figure out how many input channels we need to open.
    if (m_audioInputs.empty()) {
        m_inputParams.channelCount = 0;
        pInputParams = nullptr;
    } else {
        foreach (AudioInput in, m_audioInputs) {
            ChannelGroup channelGroup = in.getChannelGroup();
            int highChannel = channelGroup.getChannelBase()
                + channelGroup.getChannelCount();
            if (m_inputParams.channelCount <= highChannel) {
                m_inputParams.channelCount = highChannel;
            }
        }
    }

    // Workaround for Bug #900364. The PortAudio ALSA hostapi opens the minimum
    // number of device channels supported by the device regardless of our
    // channel request. It has no way of notifying us when it does this. The
    // typical case this happens is when we are opening a device in mono when it
    // supports a minimum of stereo. To work around this, simply open the device
    // in stereo and only take the first channel.
    // TODO(rryan): Remove once PortAudio has a solution built in (and
    // released).
    if (m_deviceInfo->hostApi == paALSA) {
        // Only engage workaround if the device has enough input and output
        // channels.
        if (m_deviceInfo->maxInputChannels >= 2 &&
                m_inputParams.channelCount == 1) {
            m_inputParams.channelCount = 2;
        }
        if (m_deviceInfo->maxOutputChannels >= 2 &&
                m_outputParams.channelCount == 1) {
            m_outputParams.channelCount = 2;
        }
    }


    // Sample rate
    if (m_dSampleRate <= 0) {
        m_dSampleRate = kDefaultSampleRate;
    }

    qCInfo(mixxxLogDevicePortAudio)
             << "Number of output channels:"
             << m_outputParams.channelCount;
    qCInfo(mixxxLogDevicePortAudio)
             << "Number of input channels:"
             << m_inputParams.channelCount;
    qCInfo(mixxxLogDevicePortAudio)
            << "Requested sample rate:"
            << m_dSampleRate
            << "Hz";
    qCInfo(mixxxLogDevicePortAudio)
            << "Number of sample frames per buffer:"
            << m_framesPerBuffer;
    // Buffer latency
    double bufferSizeSecs = m_framesPerBuffer / m_dSampleRate;
    qCInfo(mixxxLogDevicePortAudio)
            << "Latency per buffer:"
             << bufferSizeSecs * 1000
             << "ms";

    // PortAudio's JACK backend also only properly supports
    // paFramesPerBufferUnspecified in non-blocking mode because the latency
    // comes from the JACK daemon. (PA should give an error or something though,
    // but it doesn't.)
    if (m_deviceInfo->hostApi == paJACK) {
        m_framesPerBuffer = paFramesPerBufferUnspecified;
    }

    //Fill out the rest of the info.
    m_outputParams.device = m_deviceId.portAudioIndex;
    m_outputParams.sampleFormat = paFloat32;
    m_outputParams.suggestedLatency = bufferSizeSecs;
    m_outputParams.hostApiSpecificStreamInfo = nullptr;

    m_inputParams.device  = m_deviceId.portAudioIndex;
    m_inputParams.sampleFormat  = paFloat32;
    m_inputParams.suggestedLatency = bufferSizeSecs;
    m_inputParams.hostApiSpecificStreamInfo = nullptr;

    qCInfo(mixxxLogDevicePortAudio)
            << "Opening stream"
            << m_deviceId.portAudioIndex;

    m_lastCallbackEntrytoDacSecs = bufferSizeSecs;

    m_syncBuffers = syncBuffers;

    // Create the callback function pointer.
    PaStreamCallback* callback = nullptr;
    if (isClkRefDevice) {
        callback = paV19CallbackClkRef;
    } else if (m_syncBuffers == 2) { // "Default (long delay)"
        callback = paV19CallbackDrift;
        // to avoid overflows when one callback overtakes the other or
        // when there is a clock drift compared to the clock reference device
        // we need an additional artificial delay
        if (m_outputParams.channelCount) {
            // On chunk for reading one for writing and on for drift correction
            DEBUG_ASSERT(!m_outputFifo);
            m_outputFifo = new FIFO<CSAMPLE>(
                    m_outputParams.channelCount * m_framesPerBuffer
                            * kFifoSize);
            // Clear first 1.5 chunks on for the required artificial delaly to
            // a allow jitter and a half, because we can't predict which
            // callback fires first.
            int writeCount = m_outputParams.channelCount * m_framesPerBuffer *
                    kFifoSize / 2;
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;
            (void) m_outputFifo->aquireWriteRegions(writeCount, &dataPtr1,
                    &size1, &dataPtr2, &size2);
            SampleUtil::clear(dataPtr1, size1);
            SampleUtil::clear(dataPtr2, size2);
            (void) m_outputFifo->releaseWriteRegions(writeCount);
        }
        if (m_inputParams.channelCount) {
            DEBUG_ASSERT(!m_inputFifo);
            m_inputFifo = new FIFO<CSAMPLE>(
                    m_inputParams.channelCount * m_framesPerBuffer * kFifoSize);
            // Clear first 1.5 chunks (see above)
            int writeCount = m_inputParams.channelCount * m_framesPerBuffer *
                    kFifoSize / 2;
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;
            (void) m_inputFifo->aquireWriteRegions(writeCount, &dataPtr1,
                    &size1, &dataPtr2, &size2);
            SampleUtil::clear(dataPtr1, size1);
            SampleUtil::clear(dataPtr2, size2);
            (void) m_inputFifo->releaseWriteRegions(writeCount);
        }
    } else if (m_syncBuffers == 1) { // "Disabled (short delay)"
        // this can be used on a second device when it is driven by the Clock
        // reference device clock
        callback = paV19Callback;
        if (m_outputParams.channelCount) {
            DEBUG_ASSERT(!m_outputFifo);
            m_outputFifo = new FIFO<CSAMPLE>(
                    m_outputParams.channelCount * m_framesPerBuffer);
        }
        if (m_inputParams.channelCount) {
            DEBUG_ASSERT(!m_inputFifo);
            m_inputFifo = new FIFO<CSAMPLE>(
                    m_inputParams.channelCount * m_framesPerBuffer);
        }
    } else if (m_syncBuffers == 0) { // "Experimental (no delay)"
        if (m_outputParams.channelCount) {
            DEBUG_ASSERT(!m_outputFifo);
            m_outputFifo = new FIFO<CSAMPLE>(
                    m_outputParams.channelCount * m_framesPerBuffer * 2);
        }
        if (m_inputParams.channelCount) {
            DEBUG_ASSERT(!m_inputFifo);
            m_inputFifo = new FIFO<CSAMPLE>(
                    m_inputParams.channelCount * m_framesPerBuffer * 2);
        }
    }

    PaStream* pStream{};
    // Try open device using iChannelMax
    err = Pa_OpenStream(&pStream,
                        pInputParams,
                        pOutputParams,
                        m_dSampleRate,
                        m_framesPerBuffer,
                        paClipOff, // Stream flags
                        callback,
                        this); // pointer passed to the callback function

    if (err != paNoError) {
        qCWarning(mixxxLogDevicePortAudio)
                << m_deviceId
                << "Error opening stream:"
                << Pa_GetErrorText(err);
        m_lastError = QString::fromUtf8(Pa_GetErrorText(err));
        return SOUNDDEVICE_ERROR_ERR;
    }
    qCDebug(mixxxLogDevicePortAudio)
            << m_deviceId
            << "Opened stream";

#ifdef __LINUX__
    if (m_deviceInfo->hostApi == paALSA) {
        PaAlsa_EnableRealtimeScheduling(pStream, 1);
    }
#endif

    // Start stream
    err = Pa_StartStream(pStream);
    if (err != paNoError) {
        qCWarning(mixxxLogDevicePortAudio)
                << m_deviceId
                << "Start stream error:"
                << Pa_GetErrorText(err);
        m_lastError = QString::fromUtf8(Pa_GetErrorText(err));
        err = Pa_CloseStream(pStream);
        if (err != paNoError) {
            qCWarning(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "Close stream error:"
                    << Pa_GetErrorText(err);
        }
        return SOUNDDEVICE_ERROR_ERR;
    } else {
        qCDebug(mixxxLogDevicePortAudio)
                << m_deviceId
                << "Stream started";
    }

    // Get the actual details of the stream & update Mixxx's data
    const PaStreamInfo* streamDetails = Pa_GetStreamInfo(pStream);
    m_dSampleRate = streamDetails->sampleRate;
    qCInfo(mixxxLogDevicePortAudio)
            << "Actual sample rate:"
            << m_dSampleRate
            << "Hz";
    double outputLatencyMillis = streamDetails->outputLatency * 1000;
    qCInfo(mixxxLogDevicePortAudio)
            << "Actual output latency:"
            << outputLatencyMillis
            << "ms";

    if (isClkRefDevice) {
        // Update the samplerate and latency ControlObjects, which allow the
        // waveform view to properly correct for the latency.
        ControlObject::set(ConfigKey("[Master]", "latency"), outputLatencyMillis);
        ControlObject::set(ConfigKey("[Master]", "samplerate"), m_dSampleRate);
        ControlObject::set(ConfigKey("[Master]", "audio_buffer_size"), bufferSizeSecs * 1000);
        m_invalidTimeInfoCount = 0;
        m_clkRefTimer.start();
    }
    m_pStream = pStream;
    return SOUNDDEVICE_ERROR_OK;
}

bool SoundDevicePortAudio::isOpen() const {
    return m_pStream != nullptr;
}

SoundDeviceError SoundDevicePortAudio::close() {
    qCInfo(mixxxLogDevicePortAudio)
            << "Closing stream"
            << m_deviceId.portAudioIndex;
    PaStream* pStream = m_pStream;
    m_pStream = nullptr;
    if (pStream) {
        // Make sure the stream is not stopped before we try stopping it.
        PaError err = Pa_IsStreamStopped(pStream);
        // 1 means the stream is stopped. 0 means active.
        if (err == 1) {
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "Stream already stopped";
            return SOUNDDEVICE_ERROR_OK;
        }
        // Real PaErrors are always negative.
        if (err < 0) {
            qCWarning(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "Stream already stopped:"
                    << Pa_GetErrorText(err) ;
            return SOUNDDEVICE_ERROR_ERR;
        }

        //Stop the stream.
        qCInfo(mixxxLogDevicePortAudio)
                << "Stopping stream"
                << m_deviceId.portAudioIndex;
        err = Pa_StopStream(pStream);

        //Trying Pa_AbortStream instead, because StopStream seems to wait
        //until all the buffers have been flushed, which can take a
        //few (annoying) seconds when you're doing soundcard input.
        //(it flushes the input buffer, and then some, or something)
        //BIG FAT WARNING: Pa_AbortStream() will kill threads while they're
        //waiting on a mutex, which will leave the mutex in an screwy
        //state. Don't use it!
        //PaError err = Pa_AbortStream(m_pStream);

        if (err != paNoError) {
            qCWarning(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "Stop stream error:"
                    << Pa_GetErrorText(err);
            return SOUNDDEVICE_ERROR_ERR;
        }
        qCDebug(mixxxLogDevicePortAudio)
                << m_deviceId
                << "Stream stopped";

        // Close stream
        qCInfo(mixxxLogDevicePortAudio)
                << "Closing stream"
                << m_deviceId.portAudioIndex;
        err = Pa_CloseStream(pStream);
        if (err != paNoError) {
            qCWarning(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "Close stream error:"
                    << Pa_GetErrorText(err);
            return SOUNDDEVICE_ERROR_ERR;
        }
        qCDebug(mixxxLogDevicePortAudio)
                << m_deviceId
                << "Stream closed";

        if (m_outputFifo) {
            delete m_outputFifo;
            m_outputFifo = nullptr;
        }
        if (m_inputFifo) {
            delete m_inputFifo;
            m_inputFifo = nullptr;
        }
    }
    DEBUG_ASSERT(!m_outputFifo);
    DEBUG_ASSERT(!m_inputFifo);

    m_bSetThreadPriority = false;

    return SOUNDDEVICE_ERROR_OK;
}

QString SoundDevicePortAudio::getError() const {
    return m_lastError;
}

void SoundDevicePortAudio::readProcess() {
    if (m_pStream && m_inputParams.channelCount && m_inputFifo) {
        const int inChunkSize = m_framesPerBuffer * m_inputParams.channelCount;
        if (m_syncBuffers == 0) { // "Experimental (no delay)"
            if (m_inputFifo->readAvailable() == 0) {
                // Initial call or underflow at last call
                // Init half of the buffer with silence
                CSAMPLE* dataPtr1;
                ring_buffer_size_t size1;
                CSAMPLE* dataPtr2;
                ring_buffer_size_t size2;
                (void)m_inputFifo->aquireWriteRegions(inChunkSize,
                        &dataPtr1,
                        &size1,
                        &dataPtr2,
                        &size2);
                // Fetch fresh samples and write to the the input buffer
                SampleUtil::clear(dataPtr1, size1);
                if (size2 > 0) {
                    SampleUtil::clear(dataPtr2, size2);
                }
                (void)m_inputFifo->releaseWriteRegions(inChunkSize);
            }

            // Polling mode
            const int writeAvailable = m_inputFifo->writeAvailable();
            const int readAvailable = Pa_GetStreamReadAvailable(m_pStream) * m_inputParams.channelCount;
            const int copyCount = qMin(writeAvailable, readAvailable);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "readProcess() skip one frame"
                    << static_cast<float>(writeAvailable) / inChunkSize
                    << static_cast<float>(readAvailable) / inChunkSize;
#endif // ENABLE_TRACE_LOG
            if (copyCount > 0) {
                CSAMPLE* dataPtr1;
                ring_buffer_size_t size1;
                CSAMPLE* dataPtr2;
                ring_buffer_size_t size2;
                (void)m_inputFifo->aquireWriteRegions(copyCount,
                        &dataPtr1,
                        &size1,
                        &dataPtr2,
                        &size2);
                // Fetch fresh samples and write to the the input buffer
                PaError err = Pa_ReadStream(m_pStream, dataPtr1, size1 / m_inputParams.channelCount);
                CSAMPLE* lastFrame = &dataPtr1[size1 - m_inputParams.channelCount];
                if (err == paInputOverflowed) {
#if ENABLE_TRACE_LOG
                    qCDebug(mixxxLogDevicePortAudio)
                            << m_deviceId
                            << "readProcess() Pa_ReadStream paInputOverflowed";
#endif // ENABLE_TRACE_LOG
                    m_pSoundManager->underflowHappened(12);
                }
                if (size2 > 0) {
                    PaError err = Pa_ReadStream(m_pStream, dataPtr2, size2 / m_inputParams.channelCount);
                    lastFrame = &dataPtr2[size2 - m_inputParams.channelCount];
                    if (err == paInputOverflowed) {
#if ENABLE_TRACE_LOG
                        qCDebug(mixxxLogDevicePortAudio)
                                << m_deviceId
                                << "readProcess() Pa_ReadStream paInputOverflowed";
#endif // ENABLE_TRACE_LOG
                        m_pSoundManager->underflowHappened(13);
                    }
                }
                (void)m_inputFifo->releaseWriteRegions(copyCount);

                if (readAvailable > writeAvailable + inChunkSize / 2) {
                    // we are not able to consume enough frames
                    if (m_inputDrift) {
                        // Skip one frame
#if ENABLE_TRACE_LOG
                        qCDebug(mixxxLogDevicePortAudio)
                                << m_deviceId
                                << "readProcess() skip one frame"
                                << static_cast<float>(writeAvailable) / inChunkSize
                                << static_cast<float>(readAvailable) / inChunkSize;
#endif // ENABLE_TRACE_LOG
                        PaError err = Pa_ReadStream(m_pStream, dataPtr1, 1);
                        if (err == paInputOverflowed) {
#if ENABLE_TRACE_LOG
                            qCDebug(mixxxLogDevicePortAudio)
                                    << m_deviceId
                                    << "readProcess() Pa_ReadStream paInputOverflowed";
#endif // ENABLE_TRACE_LOG
                            m_pSoundManager->underflowHappened(14);
                        }
                    } else {
                        m_inputDrift = true;
                    }
                } else if (readAvailable < inChunkSize / 2 ||
                        m_inputFifo->readAvailable() < inChunkSize * 1.5) {
                    // We should read at least a half inChunkSize
                    // and our m_iputFifo should now hold a half chunk extra
                    if (m_inputDrift) {
                        // duplicate one frame
#if ENABLE_TRACE_LOG
                        qCDebug(mixxxLogDevicePortAudio)
                                << m_deviceId
                                << "readProcess() duplicate one frame"
                                << static_cast<float>(writeAvailable) / inChunkSize
                                << static_cast<float>(readAvailable) / inChunkSize;
#endif // ENABLE_TRACE_LOG
                        (void)m_inputFifo->aquireWriteRegions(
                                m_inputParams.channelCount, &dataPtr1, &size1, &dataPtr2, &size2);
                        if (size1) {
                            SampleUtil::copy(dataPtr1, lastFrame, size1);
                            (void)m_inputFifo->releaseWriteRegions(size1);
                        }
                    } else {
                        m_inputDrift = true;
                    }
                } else {
                    m_inputDrift = false;
                }
            }
        }

        const int readAvailable = m_inputFifo->readAvailable();
        int readCount = inChunkSize;
        if (inChunkSize > readAvailable) {
            readCount = readAvailable;
            m_pSoundManager->underflowHappened(15);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "readProcess() underflow"
                    << static_cast<float>(readAvailable) / inChunkSize;
#endif // ENABLE_TRACE_LOG
        }
#if ENABLE_TRACE_LOG
        qCDebug(mixxxLogDevicePortAudio)
                << m_deviceId
                << "readProcess()"
                << static_cast<float>(readAvailable) / inChunkSize;
#endif // ENABLE_TRACE_LOG
        if (readCount) {
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;
            // We use size1 and size2, so we can ignore the return value
            (void)m_inputFifo->aquireReadRegions(readCount, &dataPtr1, &size1, &dataPtr2, &size2);
            // Fetch fresh samples and write to the the output buffer
            composeInputBuffer(dataPtr1,
                    size1 / m_inputParams.channelCount,
                    0,
                    m_inputParams.channelCount);
            if (size2 > 0) {
                composeInputBuffer(dataPtr2,
                        size2 / m_inputParams.channelCount,
                        size1 / m_inputParams.channelCount,
                        m_inputParams.channelCount);
            }
            (void)m_inputFifo->releaseReadRegions(readCount);
        }
        if (readCount < inChunkSize) {
            // Fill remaining buffers with zeros
            clearInputBuffer(inChunkSize - readCount, readCount);
        }

        m_pSoundManager->pushInputBuffers(m_audioInputs, m_framesPerBuffer);
    }
}

void SoundDevicePortAudio::writeProcess() {
    PaStream* pStream = m_pStream;

    if (pStream && m_outputParams.channelCount && m_outputFifo) {
        const int outChunkSize = m_framesPerBuffer * m_outputParams.channelCount;
        const int writeAvailable = m_outputFifo->writeAvailable();
        int writeCount = outChunkSize;
        if (outChunkSize > writeAvailable) {
            writeCount = writeAvailable;
            m_pSoundManager->underflowHappened(16);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "writeProcess()"
                    << static_cast<float>(writeAvailable) / outChunkSize
                    << "Overflow";
#endif // ENABLE_TRACE_LOG
        }
        if (writeCount > 0) {
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;
            // We use size1 and size2, so we can ignore the return value
            (void)m_outputFifo->aquireWriteRegions(writeCount, &dataPtr1, &size1, &dataPtr2, &size2);
            // Fetch fresh samples and write to the the output buffer
            composeOutputBuffer(dataPtr1, size1 / m_outputParams.channelCount, 0, m_outputParams.channelCount);
            if (size2 > 0) {
                composeOutputBuffer(dataPtr2,
                        size2 / m_outputParams.channelCount,
                        size1 / m_outputParams.channelCount,
                        m_outputParams.channelCount);
            }
            m_outputFifo->releaseWriteRegions(writeCount);
        }

        if (m_syncBuffers == 0) { // "Experimental (no delay)"
            // Polling
            const int writeAvailable = Pa_GetStreamWriteAvailable(pStream) * m_outputParams.channelCount;
            const int readAvailable = m_outputFifo->readAvailable();
            int copyCount = qMin(writeAvailable, readAvailable);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "writeProcess()"
                    << static_cast<float>(writeAvailable) / outChunkSize
                    << static_cast<float>(readAvailable) / outChunkSize;
#endif // ENABLE_TRACE_LOG
            if (copyCount > 0) {
                CSAMPLE* dataPtr1;
                ring_buffer_size_t size1;
                CSAMPLE* dataPtr2;
                ring_buffer_size_t size2;
                m_outputFifo->aquireReadRegions(copyCount,
                        &dataPtr1,
                        &size1,
                        &dataPtr2,
                        &size2);
                if (writeAvailable >= outChunkSize * 2) {
                    // Underflow (2 is max for native ALSA devices)
                    // fill buffer with duplicate of first sample
                    for (int i = 0; i < writeAvailable - copyCount;
                            i += m_outputParams.channelCount) {
                        Pa_WriteStream(pStream, dataPtr1, 1);
                    }
#if ENABLE_TRACE_LOG
                    qCDebug(mixxxLogDevicePortAudio)
                            << m_deviceId
                            << "writeProcess() fill buffer"
                            << static_cast<float>(writeAvailable - copyCount) / outChunkSize;
#endif // ENABLE_TRACE_LOG
                    m_pSoundManager->underflowHappened(17);
                } else if (writeAvailable > readAvailable + outChunkSize / 2) {
                    // try to keep PAs buffer filled up to 0.5 chunks
                    if (m_outputDrift) {
                        // duplicate one frame
#if ENABLE_TRACE_LOG
                        qCDebug(mixxxLogDevicePortAudio)
                                << m_deviceId
                                << "writeProcess() duplicate one frame"
                                << static_cast<float>(writeAvailable) / outChunkSize
                                << static_cast<float>(readAvailable) / outChunkSize;
#endif // ENABLE_TRACE_LOG
                        PaError err = Pa_WriteStream(pStream, dataPtr1, 1);
                        if (err == paOutputUnderflowed) {
#if ENABLE_TRACE_LOG
                            qCDebug(mixxxLogDevicePortAudio)
                                    << m_deviceId
                                    << "writeProcess() Pa_ReadStream paOutputUnderflowed";
#endif // ENABLE_TRACE_LOG
                            m_pSoundManager->underflowHappened(18);
                        }
                    } else {
#if ENABLE_TRACE_LOG
                        qCDebug(mixxxLogDevicePortAudio)
                                << m_deviceId
                                << "writeProcess() OK"
                                << static_cast<float>(writeAvailable) / outChunkSize
                                << static_cast<float>(readAvailable) / outChunkSize;
#endif // ENABLE_TRACE_LOG
                        m_outputDrift = true;
                    }
                } else if (writeAvailable < outChunkSize / 2 ||
                        readAvailable - copyCount > outChunkSize * 0.5) {
                    // We are not able to store at least the half of the new frames
                    // or we have a risk of an m_outputFifo overflow
                    if (m_outputDrift) {
#if ENABLE_TRACE_LOG
                        qCDebug(mixxxLogDevicePortAudio)
                                << m_deviceId
                                << "writeProcess() skip one frame"
                                << static_cast<float>(writeAvailable) / outChunkSize
                                << static_cast<float>(readAvailable) / outChunkSize;
#endif // ENABLE_TRACE_LOG
                        copyCount = qMin(readAvailable, copyCount + m_iNumOutputChannels);
                    } else {
                        m_outputDrift = true;
                    }
                } else {
                    m_outputDrift = false;
                }
                PaError err = Pa_WriteStream(pStream, dataPtr1, size1 / m_outputParams.channelCount);
                if (err == paOutputUnderflowed) {
#if ENABLE_TRACE_LOG
                    qCDebug(mixxxLogDevicePortAudio)
                            << m_deviceId
                            << "writeProcess() Pa_ReadStream paOutputUnderflowed";
#endif // ENABLE_TRACE_LOG
                    m_pSoundManager->underflowHappened(19);
                }
                if (size2 > 0) {
                    PaError err = Pa_WriteStream(pStream, dataPtr2, size2 / m_outputParams.channelCount);
                    if (err == paOutputUnderflowed) {
#if ENABLE_TRACE_LOG
                        qCDebug(mixxxLogDevicePortAudio)
                                << m_deviceId
                                << "writeProcess() Pa_WriteStream paOutputUnderflowed";
#endif // ENABLE_TRACE_LOG
                        m_pSoundManager->underflowHappened(20);
                    }
                }
                m_outputFifo->releaseReadRegions(copyCount);
            }
        }
    }
}

int SoundDevicePortAudio::callbackProcessDrift(
        const SINT framesPerBuffer, CSAMPLE *out, const CSAMPLE *in,
        const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags statusFlags) {
    Q_UNUSED(timeInfo);
    Trace trace("SoundDevicePortAudio::callbackProcessDrift %1",
            m_deviceId.debugName());

    if (statusFlags & (paOutputUnderflow | paInputOverflow)) {
        m_pSoundManager->underflowHappened(7);
    }

    // Since we are on the non Clock reference device and may have an independent
    // Crystal clock, a drift correction is required
    //
    // There is a delay of up to one latency between composing a chunk in the Clock
    // Reference callback and write it to the device. So we need at lest one buffer.
    // Unfortunately this delay is somehow random, an WILL produce a delay slow
    // shift without we can avoid it. (That's the price for using a cheap USB soundcard).
    //
    // Additional we need an filled chunk and an empty chunk. These are used when on
    // sound card overtakes the other. This always happens, if they are driven form
    // two crystals. In a test case every 30 s @ 23 ms. After they are consumed,
    // the drift correction takes place and fills or clears the reserve buffers.
    // If this is finished before another overtake happens, we do not face any
    // dropouts or clicks.
    // So that's why we need a Fifo of 3 chunks.
    //
    // In addition there is a jitter effect. It happens that one callback is delayed,
    // in this case the second one fires two times and then the first one fires two
    // time as well to catch up. This is also fixed by the additional buffers. If this
    // happens just after an regular overtake, we will have clicks again.
    //
    // I the tests it turns out that it only happens in the opposite direction, so
    // 3 chunks are just fine.

    if (m_inputParams.channelCount) {
        const int inChunkSize = framesPerBuffer * m_inputParams.channelCount;
        const int writeAvailable = m_inputFifo->writeAvailable();
        const int readAvailable = m_inputFifo->readAvailable();
        if (readAvailable < inChunkSize * kDriftReserve) {
            // risk of an underflow, duplicate one frame
            (void) m_inputFifo->write(in, inChunkSize);
            if (m_inputDrift) {
                // Do not compensate the first delay, because it is likely a jitter
                // corrected in the next cycle
                // Duplicate one frame
                (void) m_inputFifo->write(
                        &in[inChunkSize - m_inputParams.channelCount],
                        m_inputParams.channelCount);
#if ENABLE_TRACE_LOG
                qCDebug(mixxxLogDevicePortAudio)
                        << m_deviceId
                        << "callbackProcessDrift write:"
                        << static_cast<float>(readAvailable) / inChunkSize;
#endif // ENABLE_TRACE_LOG
            } else {
                m_inputDrift = true;
#if ENABLE_TRACE_LOG
                qCDebug(mixxxLogDevicePortAudio)
                        << m_deviceId
                        << "callbackProcessDrift write:"
                        << static_cast<float>(readAvailable) / inChunkSize
                        << "Jitter Skip";
#endif // ENABLE_TRACE_LOG
            }
        } else if (readAvailable == inChunkSize * kDriftReserve) {
            // Everything Ok
            (void) m_inputFifo->write(in, inChunkSize);
            m_inputDrift = false;
#if ENABLE_TRACE_LOG
                qCDebug(mixxxLogDevicePortAudio)
                        << m_deviceId
                        << "callbackProcess write:"
                        << static_cast<float>(readAvailable) / inChunkSize
                        << "Normal";
#endif // ENABLE_TRACE_LOG
        } else if (writeAvailable >= inChunkSize) {
            // Risk of overflow, skip one frame
            if (m_inputDrift) {
                (void) m_inputFifo->write(in, inChunkSize - m_inputParams.channelCount);
#if ENABLE_TRACE_LOG
                qCDebug(mixxxLogDevicePortAudio)
                        << m_deviceId
                        << "callbackProcessDrift write:"
                        << static_cast<float>(readAvailable) / inChunkSize
                        << "Skip";
#endif // ENABLE_TRACE_LOG
            } else {
                (void) m_inputFifo->write(in, inChunkSize);
                m_inputDrift = true;
#if ENABLE_TRACE_LOG
                qCDebug(mixxxLogDevicePortAudio)
                        << m_deviceId
                        << "callbackProcessDrift write:"
                        << static_cast<float>(readAvailable) / inChunkSize
                        << "Jitter Skip";
#endif // ENABLE_TRACE_LOG
            }
        } else if (writeAvailable) {
            // Fifo Overflow
            (void)m_inputFifo->write(in, writeAvailable);
            m_pSoundManager->underflowHappened(8);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "callbackProcessDrift write:"
                    << static_cast<float>(readAvailable) / inChunkSize
                    << "Overflow";
#endif // ENABLE_TRACE_LOG
        } else {
            // Buffer full
            m_pSoundManager->underflowHappened(9);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "callbackProcessDrift write:"
                    << static_cast<float>(readAvailable) / inChunkSize
                    << "Buffer full";
#endif // ENABLE_TRACE_LOG
        }
    }

    if (m_outputParams.channelCount) {
        const int outChunkSize = framesPerBuffer * m_outputParams.channelCount;
        const int readAvailable = m_outputFifo->readAvailable();

        if (readAvailable > outChunkSize * (kDriftReserve + 1)) {
            (void) m_outputFifo->read(out, outChunkSize);
            if (m_outputDrift) {
                // Risk of overflow, skip one frame
                (void)m_outputFifo->releaseReadRegions(m_outputParams.channelCount);
#if ENABLE_TRACE_LOG
                qCDebug(mixxxLogDevicePortAudio)
                        << m_deviceId
                        << "callbackProcessDrift read:"
                        << static_cast<float>(readAvailable) / outChunkSize
                        << "Skip";
#endif // ENABLE_TRACE_LOG
            } else {
                m_outputDrift = true;
#if ENABLE_TRACE_LOG
                qCDebug(mixxxLogDevicePortAudio)
                        << m_deviceId
                        << "callbackProcessDrift read:"
                        << static_cast<float>(readAvailable) / outChunkSize
                        << "Jitter Skip";
#endif // ENABLE_TRACE_LOG
            }
        } else if (readAvailable == outChunkSize * (kDriftReserve + 1)) {
            (void) m_outputFifo->read(out, outChunkSize);
            m_outputDrift = false;
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "callbackProcessDrift read:"
                    << static_cast<float>(readAvailable) / outChunkSize
                    << "Normal";
#endif // ENABLE_TRACE_LOG
        } else if (readAvailable >= outChunkSize) {
            if (m_outputDrift) {
                // Risk of underflow, duplicate one frame
                (void)m_outputFifo->read(out,
                        outChunkSize - m_outputParams.channelCount);
                SampleUtil::copy(
                        &out[outChunkSize - m_outputParams.channelCount],
                        &out[outChunkSize - (2 * m_outputParams.channelCount)],
                        m_outputParams.channelCount);
#if ENABLE_TRACE_LOG
                qCDebug(mixxxLogDevicePortAudio)
                        << m_deviceId
                        << "callbackProcessDrift read:"
                        << static_cast<float>(readAvailable) / outChunkSize
                        << "Save";
#endif // ENABLE_TRACE_LOG
            } else {
                (void) m_outputFifo->read(out, outChunkSize);
                m_outputDrift = true;
#if ENABLE_TRACE_LOG
                qCDebug(mixxxLogDevicePortAudio)
                        << m_deviceId
                        << "callbackProcessDrift read:"
                        << static_cast<float>(readAvailable) / outChunkSize
                        << "Jitter Save";
#endif // ENABLE_TRACE_LOG
            }
        } else if (readAvailable) {
            (void) m_outputFifo->read(out,
                    readAvailable);
            // underflow
            SampleUtil::clear(&out[readAvailable],
                    outChunkSize - readAvailable);
            m_pSoundManager->underflowHappened(10);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "callbackProcessDrift read:"
                    << static_cast<float>(readAvailable) / outChunkSize
                    << "Underflow";
#endif // ENABLE_TRACE_LOG
        } else {
            // underflow (buffer empty)
            SampleUtil::clear(out, outChunkSize);
            m_pSoundManager->underflowHappened(11);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "callbackProcessDrift read:"
                    << static_cast<float>(readAvailable) / outChunkSize
                    << "Buffer empty";
#endif // ENABLE_TRACE_LOG
        }
     }
    return paContinue;
}

int SoundDevicePortAudio::callbackProcess(const SINT framesPerBuffer,
        CSAMPLE *out, const CSAMPLE *in,
        const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags statusFlags) {
    Q_UNUSED(timeInfo);
    Trace trace("SoundDevicePortAudio::callbackProcess %1", m_deviceId.debugName());

    if (statusFlags & (paOutputUnderflow | paInputOverflow)) {
        m_pSoundManager->underflowHappened(1);
#if ENABLE_TRACE_LOG
        qCDebug(mixxxLogDevicePortAudio)
                << m_deviceId
                << "callbackProcess read:"
                << "Underflow";
#endif // ENABLE_TRACE_LOG
    }

    if (m_inputParams.channelCount) {
        const int inChunkSize = framesPerBuffer * m_inputParams.channelCount;
        const int writeAvailable = m_inputFifo->writeAvailable();
        if (writeAvailable >= inChunkSize) {
            (void) m_inputFifo->write(in, inChunkSize - m_inputParams.channelCount);
        } else if (writeAvailable) {
            // Fifo Overflow
            (void)m_inputFifo->write(in, writeAvailable);
            m_pSoundManager->underflowHappened(2);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "callbackProcess write:"
                    << "Overflow";
#endif // ENABLE_TRACE_LOG
        } else {
            // Buffer full
            m_pSoundManager->underflowHappened(3);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "callbackProcess write:"
                    << "Buffer full";
#endif // ENABLE_TRACE_LOG
        }
    }

    if (m_outputParams.channelCount) {
        const int outChunkSize = framesPerBuffer * m_outputParams.channelCount;
        const int readAvailable = m_outputFifo->readAvailable();
        if (readAvailable >= outChunkSize) {
            (void) m_outputFifo->read(out, outChunkSize);
        } else if (readAvailable) {
            (void) m_outputFifo->read(out,
                    readAvailable);
            // underflow
            SampleUtil::clear(&out[readAvailable],
                    outChunkSize - readAvailable);
            m_pSoundManager->underflowHappened(4);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "callbackProcess read:"
                    << "Underflow";
#endif // ENABLE_TRACE_LOG
        } else {
            // underflow (buffer empty)
            SampleUtil::clear(out, outChunkSize);
            m_pSoundManager->underflowHappened(5);
#if ENABLE_TRACE_LOG
            qCDebug(mixxxLogDevicePortAudio)
                    << m_deviceId
                    << "callbackProcess read:"
                    << "Buffer empty";
#endif // ENABLE_TRACE_LOG
        }
     }
    return paContinue;
}

int SoundDevicePortAudio::callbackProcessClkRef(
        const SINT framesPerBuffer, CSAMPLE *out, const CSAMPLE *in,
        const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags statusFlags) {
    // This must be the very first call, else timeInfo becomes invalid
    updateCallbackEntryToDacTime(timeInfo);

    Trace trace("SoundDevicePortAudio::callbackProcessClkRef %1",
                m_deviceId.debugName());
#if ENABLE_TRACE_LOG
    qCDebug(mixxxLogDevicePortAudio)
            << m_deviceId
            << "callbackProcessClkRef";
#endif // ENABLE_TRACE_LOG

    // Turn on TimeCritical priority for the callback thread. If we are running
    // in Linux userland, for example, this will have no effect.
    if (!m_bSetThreadPriority) {
        QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);
        m_bSetThreadPriority = true;


#ifdef __SSE__
        // This disables the denormals calculations, to avoid a
        // performance penalty of ~20
        // https://bugs.launchpad.net/mixxx/+bug/1404401
        if (!_MM_GET_DENORMALS_ZERO_MODE()) {
            qCInfo(mixxxLogDevicePortAudio)
                    << "SSE: Enabling denormals to zero mode";
            _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
        } else {
             qCInfo(mixxxLogDevicePortAudio)
                    << "SSE: Denormals to zero mode already enabled";
        }

        if (!_MM_GET_FLUSH_ZERO_MODE()) {
            qCInfo(mixxxLogDevicePortAudio)
                    << "SSE: Enabling flush to zero mode";
            _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
        } else {
             qCInfo(mixxxLogDevicePortAudio)
                    << "SSE: Flush to zero mode already enabled";
        }
#else
#if defined( __i386__ ) || defined( __i486__ ) || defined( __i586__ ) || \
         defined( __i686__ ) || defined( __x86_64__ ) || defined (_M_I86)
        qCWarning(mixxxLogDevicePortAudio)
                << "No SSE: No denormals to zero mode available. EQs and effects may suffer high CPU load";
#endif
#endif
        // verify if flush to zero or denormals to zero works
        // test passes if one of the two flag is set.
        volatile double doubleMin = DBL_MIN; // the smallest normalized double
        VERIFY_OR_DEBUG_ASSERT(doubleMin / 2 == 0.0) {
            qCWarning(mixxxLogDevicePortAudio)
                    << "Denormals to zero mode is not working. EQs and effects may suffer high CPU load";
        } else {
            qCInfo(mixxxLogDevicePortAudio)
                    << "Denormals to zero mode is working";
        }
    }

#ifdef __SSE__
#ifdef __WINDOWS__
    // We need to refresh the denormals flags every callback since some
    // driver + API combinations will reset them (known: DirectSound + Realtec)
    // Fixes Bug #1495047
    // (Both calls are very fast)
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
#endif
#endif

    if (statusFlags & (paOutputUnderflow | paInputOverflow)) {
        m_pSoundManager->underflowHappened(6);
    }

    m_pSoundManager->processUnderflowHappened();

    //Note: Input is processed first so that any ControlObject changes made in
    //      response to input are processed as soon as possible (that is, when
    //      m_pSoundManager->requestBuffer() is called below.)

    // Send audio from the soundcard's input off to the SoundManager...
    if (in) {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess input %1",
                m_deviceId.debugName());
        composeInputBuffer(in, framesPerBuffer, 0, m_inputParams.channelCount);
        m_pSoundManager->pushInputBuffers(m_audioInputs, m_framesPerBuffer);
    }

    m_pSoundManager->readProcess();

    {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess prepare %1",
                m_deviceId.debugName());
        m_pSoundManager->onDeviceOutputCallback(framesPerBuffer);
    }

    if (out) {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess output %1",
                m_deviceId.debugName());

        if (m_outputParams.channelCount <= 0) {
            qCWarning(mixxxLogDevicePortAudio)
                    << "SoundDevicePortAudio::callbackProcess m_outputParams channel count is zero or less:"
                    << m_outputParams.channelCount;
            // Bail out.
            return paContinue;
        }

        composeOutputBuffer(out, framesPerBuffer, 0, m_outputParams.channelCount);
    }

    m_pSoundManager->writeProcess();

    updateAudioLatencyUsage(framesPerBuffer);

    return paContinue;
}

void SoundDevicePortAudio::updateCallbackEntryToDacTime(
        const PaStreamCallbackTimeInfo* timeInfo) {
    double timeSinceLastCbSecs = m_clkRefTimer.restart().toDoubleSeconds();

    // Plausibility check:
    // We have the DAC timing as reference with almost no jitter
    // (else the sound would be distorted)
    // The Callback is called with the same rate, but with a portion
    // of jitter.
    // We may get callbackEntrytoDacSecs (CED) from Portaudio or unreliable
    // rubish, depending on the underlying driver.
    // timeSinceLastCb (SLC), is the time between the callback measured by the
    // CPU Which has almost but not the same speed than the DAC timing
    //
    // DAC ---|---------|---------|---------|---------| 10 units
    // CED1           |-----------|                     12 units
    // CED2                   |-------------|           14 units
    // CED3                           |---------------| 16 units
    // SLC1|----------|                                 11 units
    // SLC2           |-------|                          8 units
    // SLC3                   |-------|                  8 units
    //
    // To check if we can trust the callbackEntrytoDacSecs (CED) value
    // this must be almost true (almost because of the DAC clock CPU clock
    // difference)
    //
    // SLC2 + CED2 = CED1 + DAC  -> 8 + 14 = 12 + 10

    PaTime callbackEntrytoDacSecs = timeInfo->outputBufferDacTime
            - timeInfo->currentTime;
    double bufferSizeSecs = m_framesPerBuffer / m_dSampleRate;


    double diff = (timeSinceLastCbSecs + callbackEntrytoDacSecs) -
            (m_lastCallbackEntrytoDacSecs + bufferSizeSecs);

    if (callbackEntrytoDacSecs <= 0 ||
            (timeSinceLastCbSecs < bufferSizeSecs * 2 &&
            fabs(diff) / bufferSizeSecs > 0.1)) {
        // Fall back to CPU timing:
        // If timeSinceLastCbSecs from a CPU timer is reasonable (no underflow),
        // the callbackEntrytoDacSecs time is not in the past
        // and we have more than 10 % difference to the timing provided by Portaudio
        // we do not trust the Portaudio timing.
        // (A difference up to ~ 5 % is normal)

        m_invalidTimeInfoCount++;

        if (m_invalidTimeInfoCount == kInvalidTimeInfoWarningCount) {
            if (CmdlineArgs::Instance().getDeveloper()) {
                qCWarning(mixxxLogDevicePortAudio)
                           << "Audio API provides invalid time stamps,"
                           << "syncing waveforms with a CPU Timer"
                           << "DacTime:" << timeInfo->outputBufferDacTime
                           << "EntrytoDac:" << callbackEntrytoDacSecs
                           << "TimeSinceLastCb:" << timeSinceLastCbSecs
                           << "diff:" << diff;
            }
        }

        callbackEntrytoDacSecs = (m_lastCallbackEntrytoDacSecs + bufferSizeSecs)
                - timeSinceLastCbSecs;
        // clamp values to avoid a big offset due to clock drift.
        callbackEntrytoDacSecs = math_clamp(callbackEntrytoDacSecs, 0.0, bufferSizeSecs * 2);
    }

    VisualPlayPosition::setCallbackEntryToDacSecs(callbackEntrytoDacSecs, m_clkRefTimer);
    m_lastCallbackEntrytoDacSecs = callbackEntrytoDacSecs;

#if ENABLE_TRACE_LOG
    qCDebug(mixxxLogDevicePortAudio)
            << callbackEntrytoDacSecs
            << timeSinceLastCbSecs;
#endif // ENABLE_TRACE_LOG
}

void SoundDevicePortAudio::updateAudioLatencyUsage(
        const SINT framesPerBuffer) {
    m_framesSinceAudioLatencyUsageUpdate += framesPerBuffer;
    if (m_framesSinceAudioLatencyUsageUpdate > (m_dSampleRate / kCpuUsageUpdateRate)) {
        double secInAudioCb = m_timeInAudioCallback.toDoubleSeconds();
        m_pMasterAudioLatencyUsage->set(
                secInAudioCb
                        / (m_framesSinceAudioLatencyUsageUpdate / m_dSampleRate));
        m_timeInAudioCallback = mixxx::Duration::fromSeconds(0);
        m_framesSinceAudioLatencyUsageUpdate = 0;
#if ENABLE_TRACE_LOG
        qCDebug(mixxxLogDevicePortAudio)
                << m_pMasterAudioLatencyUsage
                << m_pMasterAudioLatencyUsage->get();
#endif // ENABLE_TRACE_LOG
    }
    // measure time in Audio callback at the very last
    m_timeInAudioCallback += m_clkRefTimer.elapsed();
}
