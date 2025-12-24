#include "soundio/sounddevicenetwork.h"

#include <QtDebug>

#include "control/controlobject.h"
#include "engine/sidechain/enginenetworkstream.h"
#include "float.h"
#include "moc_sounddevicenetwork.cpp"
#include "soundio/sounddevice.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"
#include "util/denormalsarezero.h"
#include "util/logger.h"
#include "util/sample.h"
#include "util/timer.h"
#include "util/trace.h"
#include "waveform/visualplayposition.h"

namespace {
constexpr int kNetworkLatencyFrames = 8192; // 185 ms @ 44100 Hz
// Related chunk sizes:
// Mp3 frames = 1152 samples
// Ogg frames = 64 to 8192 samples.
// In Mixxx 1.11 we transmit every decoder-frames at once,
// Which results in case of ogg in a dynamic latency from 0.14 ms to to 185 ms
// Now we have switched to a fixed latency of 8192 frames (stereo samples) =
// which is 185 @ 44100 ms and twice the maximum of the max mixxx audio buffer

const mixxx::Logger kLogger("SoundDeviceNetwork");

const QString kAppGroup = QStringLiteral("[App]");
} // namespace

SoundDeviceNetwork::SoundDeviceNetwork(
        UserSettingsPointer config,
        SoundManager* sm,
        QSharedPointer<EngineNetworkStream> pNetworkStream)
        : SoundDevice(config, sm),
          m_pNetworkStream(pNetworkStream),
          m_inputDrift(false),
          m_audioLatencyUsage(kAppGroup, QStringLiteral("audio_latency_usage")),
          m_framesSinceAudioLatencyUsageUpdate(0),
          m_denormals(false),
          m_targetTime(0) {
    // Setting parent class members:
    m_hostAPI = "Network stream";
    m_sampleRate = SoundManagerConfig::kMixxxDefaultSampleRate;
    m_deviceId.name = kNetworkDeviceInternalName;
    m_strDisplayName = QObject::tr("Network stream");
    m_numInputChannels = pNetworkStream->getNumInputChannels();
    m_numOutputChannels = pNetworkStream->getNumOutputChannels();
}

SoundDeviceNetwork::~SoundDeviceNetwork() {
}

SoundDeviceStatus SoundDeviceNetwork::open(bool isClkRefDevice, int syncBuffers) {
    Q_UNUSED(syncBuffers);
    kLogger.debug() << "open:" << m_deviceId.name;

    // Sample rate
    if (!m_sampleRate.isValid()) {
        m_sampleRate = SoundManagerConfig::kMixxxDefaultSampleRate;
    }

    const SINT framesPerBuffer = m_configFramesPerBuffer;
    const auto requestedBufferTime = mixxx::Duration::fromSeconds(
            framesPerBuffer / m_sampleRate.toDouble());

    // Feed the network device buffer directly from the
    // clock reference device callback
    // This is what should work best.
    if (m_numOutputChannels) {
        m_outputFifo = std::make_unique<FIFO<CSAMPLE>>(
                m_numOutputChannels * framesPerBuffer * 2);
    }
    if (m_numInputChannels) {
        m_inputFifo = std::make_unique<FIFO<CSAMPLE>>(
                m_numInputChannels * framesPerBuffer * 2);
    }

    m_pNetworkStream->startStream(m_sampleRate);

    // Create the callback Thread if requested
    if (isClkRefDevice) {
        kLogger.debug() << "Clock Reference with:" << framesPerBuffer << "frames/buffer @"
                        << m_sampleRate << "Hz =" << requestedBufferTime.formatMillisWithUnit();

        // Update the samplerate and latency ControlObjects, which allow the
        // waveform view to properly correct for the latency.
        ControlObject::set(ConfigKey(kAppGroup, QStringLiteral("output_latency_ms")),
                requestedBufferTime.toDoubleMillis());
        ControlObject::set(ConfigKey(kAppGroup, QStringLiteral("samplerate")), m_sampleRate);

        // Network stream was just started above so we have to wait until
        // we can pass one chunk.
        // The first callback runs early to do the one time setups
        m_targetTime = requestedBufferTime.toIntegerMicros();

        m_pThread = std::make_unique<SoundDeviceNetworkThread>(this);
        m_pThread->start(QThread::TimeCriticalPriority);
    } else {
        kLogger.debug() << "Maximum:" << framesPerBuffer << "frames/buffer @"
                        << m_sampleRate << "Hz =" << requestedBufferTime.formatMillisWithUnit();
    }

    return SoundDeviceStatus::Ok;
}

bool SoundDeviceNetwork::isOpen() const {
    return (m_inputFifo != nullptr || m_outputFifo != nullptr);
}

SoundDeviceStatus SoundDeviceNetwork::close() {
    //kLogger.debug() << "close:" << getInternalName();
    m_pNetworkStream->stopStream();
    if (m_pThread) {
        m_pThread->stop();
        m_pThread->wait();
        m_pThread.reset();
    }

    m_outputFifo.reset();
    m_inputFifo.reset();

    return SoundDeviceStatus::Ok;
}

mixxx::audio::SampleRate SoundDeviceNetwork::getDefaultSampleRate() const {
    return SoundManagerConfig::kMixxxDefaultSampleRate;
}

QString SoundDeviceNetwork::getError() const {
    return QString();
}

void SoundDeviceNetwork::readProcess(SINT framesPerBuffer) {
    if (!m_inputFifo || !m_pNetworkStream || !m_numInputChannels.isValid()) {
        return;
    }
    DEBUG_ASSERT(m_configFramesPerBuffer >= framesPerBuffer);

    int inChunkSize = framesPerBuffer * m_numInputChannels;
    int readAvailable = m_pNetworkStream->getReadExpected() * m_numInputChannels;
    int writeAvailable = m_inputFifo->writeAvailable();
    int copyCount = qMin(writeAvailable, readAvailable);
    if (copyCount > 0) {
        CSAMPLE* dataPtr1;
        ring_buffer_size_t size1;
        CSAMPLE* dataPtr2;
        ring_buffer_size_t size2;
        (void)m_inputFifo->aquireWriteRegions(copyCount,
                &dataPtr1, &size1, &dataPtr2, &size2);
        // Fetch fresh samples and write to the the input buffer
        m_pNetworkStream->read(dataPtr1,
                size1 / m_numInputChannels);
        CSAMPLE* lastFrame = &dataPtr1[size1 - m_numInputChannels];
        if (size2 > 0) {
            m_pNetworkStream->read(dataPtr2,
                    size2 / m_numInputChannels);
            lastFrame = &dataPtr2[size2 - m_numInputChannels];
        }
        m_inputFifo->releaseWriteRegions(copyCount);

        if (readAvailable > writeAvailable + inChunkSize / 2) {
            // we are not able to consume all frames
            if (m_inputDrift) {
                // Skip one frame
                //kLogger.debug() << "readProcess() skip one frame"
                //                << (float)writeAvailable / inChunkSize << (float)readAvailable / inChunkSize;
                m_pNetworkStream->read(dataPtr1, 1);
            } else {
                m_inputDrift = true;
            }
        } else if (readAvailable < inChunkSize / 2) {
            // We should read at least inChunkSize
            if (m_inputDrift) {
                // duplicate one frame
                //kLogger.debug() << "readProcess() duplicate one frame"
                //                << (float)writeAvailable / inChunkSize << (float)readAvailable / inChunkSize;
                (void)m_inputFifo->aquireWriteRegions(
                        m_numInputChannels, &dataPtr1, &size1, &dataPtr2, &size2);
                if (size1) {
                    SampleUtil::copy(dataPtr1, lastFrame, size1);
                    m_inputFifo->releaseWriteRegions(size1);
                }
            } else {
                m_inputDrift = true;
            }
        } else {
            m_inputDrift = false;
        }
    }

    readAvailable = m_inputFifo->readAvailable();
    int readCount = inChunkSize;
    if (inChunkSize > readAvailable) {
        readCount = readAvailable;
        m_pSoundManager->underflowHappened(21);
        //qDebug() << "readProcess()" << (float)readAvailable / inChunkSize << "underflow";
    }
    if (readCount) {
        CSAMPLE* dataPtr1;
        ring_buffer_size_t size1;
        CSAMPLE* dataPtr2;
        ring_buffer_size_t size2;
        // We use size1 and size2, so we can ignore the return value
        (void) m_inputFifo->aquireReadRegions(readCount, &dataPtr1, &size1,
                &dataPtr2, &size2);
        // Fetch fresh samples and write to the the output buffer
        composeInputBuffer(dataPtr1,
                size1 / m_numInputChannels,
                0,
                m_numInputChannels);
        if (size2 > 0) {
            composeInputBuffer(dataPtr2,
                    size2 / m_numInputChannels,
                    size1 / m_numInputChannels,
                    m_numInputChannels);
        }
        m_inputFifo->releaseReadRegions(readCount);
    }
    if (readCount < inChunkSize) {
        // Fill remaining buffers with zeros
        clearInputBuffer(inChunkSize - readCount, readCount);
    }

    m_pSoundManager->pushInputBuffers(m_audioInputs, framesPerBuffer);
}

void SoundDeviceNetwork::writeProcess(SINT framesPerBuffer) {
    if (!m_outputFifo || !m_pNetworkStream || !m_numOutputChannels) {
        return;
    }
    DEBUG_ASSERT(m_configFramesPerBuffer >= framesPerBuffer);

    int outChunkSize = framesPerBuffer * m_numOutputChannels;
    int writeAvailable = m_outputFifo->writeAvailable();
    int writeCount = outChunkSize;
    if (outChunkSize > writeAvailable) {
        writeCount = writeAvailable;
        m_pSoundManager->underflowHappened(23);
        //qDebug() << "writeProcess():" << (float) writeAvailable / outChunkSize << "Overflow";
    }
    //qDebug() << "writeProcess():" << (float) writeAvailable / outChunkSize;
    if (writeCount > 0) {
        CSAMPLE* dataPtr1;
        ring_buffer_size_t size1;
        CSAMPLE* dataPtr2;
        ring_buffer_size_t size2;
        // We use size1 and size2, so we can ignore the return value
        (void)m_outputFifo->aquireWriteRegions(writeCount, &dataPtr1,
                &size1, &dataPtr2, &size2);
        // Fetch fresh samples and write to the the output buffer
        composeOutputBuffer(dataPtr1, size1 / m_numOutputChannels, 0, m_numOutputChannels);
        if (size2 > 0) {
            composeOutputBuffer(dataPtr2,
                    size2 / m_numOutputChannels,
                    size1 / m_numOutputChannels,
                    m_numOutputChannels);
        }
        m_outputFifo->releaseWriteRegions(writeCount);
    }

    int readAvailable = m_outputFifo->readAvailable();

    CSAMPLE* dataPtr1;
    ring_buffer_size_t size1;
    CSAMPLE* dataPtr2;
    ring_buffer_size_t size2;
    // Try to read as most frames as possible.
    // NetworkStreamWorker::processWrite takes care of
    // keeping every output worker in sync
    m_outputFifo->aquireReadRegions(readAvailable,
            &dataPtr1, &size1, &dataPtr2, &size2);

    const QVector<NetworkOutputStreamWorkerPtr> workers =
            m_pNetworkStream->outputWorkers();
    for (const auto& pWorker : workers) {
        if (pWorker.isNull()) {
            continue;
        }

        workerWriteProcess(pWorker,
                outChunkSize, readAvailable,
                dataPtr1, size1,
                dataPtr2, size2);
    }

    m_outputFifo->releaseReadRegions(readAvailable);
}

void SoundDeviceNetwork::workerWriteProcess(NetworkOutputStreamWorkerPtr pWorker,
        int outChunkSize, int readAvailable,
        CSAMPLE* dataPtr1, ring_buffer_size_t size1,
        CSAMPLE* dataPtr2, ring_buffer_size_t size2) {
    int writeExpectedFrames = static_cast<int>(
            pWorker->getStreamTimeFrames() - pWorker->framesWritten());

    int writeExpected = writeExpectedFrames * m_numOutputChannels;

    if (writeExpected <= 0) {
        // Overflow
        // kLogger.debug() << "workerWriteProcess: buffer full"
        //                 << "outChunkSize" << outChunkSize
        //                 << "readAvailable" << readAvailable
        //                 << "writeExpected" << writeExpected
        //                 << "streamTime" << pWorker->getStreamTimeFrames();
        // catch up by skipping chunk
        m_pSoundManager->underflowHappened(25);
    }
    int copyCount = qMin(readAvailable, writeExpected);

    if (copyCount > 0) {
        if (writeExpected - copyCount > outChunkSize) {
            // Underflow
            // kLogger.debug() << "workerWriteProcess: buffer empty."
            //                 << "Catch up with silence:" << writeExpected - copyCount
            //                 << "streamTime" << pWorker->getStreamTimeFrames();;
            // catch up by filling buffer until we are synced
            workerWriteSilence(pWorker, (writeExpected - copyCount) / m_numOutputChannels);
            m_pSoundManager->underflowHappened(24);
        } else if (writeExpected - copyCount > outChunkSize / 2) {
            // try to keep PAs buffer filled up to 0.5 chunks
            if (pWorker->outputDrift()) {
                // duplicate one frame
                // kLogger.debug() << "workerWriteProcess() duplicate one frame"
                //                 << (float)writeExpected / outChunkSize
                //                 << (float)readAvailable / outChunkSize;
                workerWrite(pWorker, dataPtr1, 1);
            } else {
                pWorker->setOutputDrift(true);
            }
        } else if (writeExpected < outChunkSize / 2) {
            // We will overshoot by more than a half of the new frames
            if (pWorker->outputDrift()) {
                // kLogger.debug() << "workerWriteProcess() "
                //                    "skip one frame"
                //                 << (float)writeAvailable / outChunkSize
                //                 << (float)readAvailable / outChunkSize;
                if (size1 >= m_numOutputChannels) {
                    dataPtr1 += m_numOutputChannels;
                    size1 -= m_numOutputChannels;
                }
            } else {
                pWorker->setOutputDrift(true);
            }
        } else {
            pWorker->setOutputDrift(false);
        }

        workerWrite(pWorker, dataPtr1, size1 / m_numOutputChannels);
        if (size2 > 0) {
            workerWrite(pWorker, dataPtr2, size2 / m_numOutputChannels);
        }

        QSharedPointer<FIFO<CSAMPLE>> pFifo = pWorker->getOutputFifo();
        if (pFifo) {
            // interval = copyCount
            // Check for desired kNetworkLatencyFrames + 1/2 interval to
            // avoid big jitter due to interferences with sync code
            if (pFifo->readAvailable() + copyCount / 2 >=
                    (m_numOutputChannels * kNetworkLatencyFrames)) {
                pWorker->outputAvailable();
            }
        }
    }
}

void SoundDeviceNetwork::workerWrite(NetworkOutputStreamWorkerPtr pWorker,
        const CSAMPLE* buffer, int frames) {
    if (!pWorker->threadWaiting()) {
        pWorker->addFramesWritten(frames);
        return;
    }

    QSharedPointer<FIFO<CSAMPLE>> pFifo = pWorker->getOutputFifo();
    if (pFifo) {
        int writeAvailable = pFifo->writeAvailable();
        int writeRequired = frames * m_numOutputChannels;
        if (writeAvailable < writeRequired) {
            kLogger.warning() << "write: worker buffer full, losing samples";
            pWorker->incOverflowCount();
        }

        int copyCount = math_min(writeAvailable, writeRequired);
        if (copyCount > 0) {
            (void)pFifo->write(buffer, copyCount);
            // we advance the frame only by the samples we have actually copied
            // This means in case of buffer full (where we loose some frames)
            // we do not get out of sync, and the syncing code tries to catch up the
            // stream by writing silence, once the buffer is free.
            pWorker->addFramesWritten(copyCount / m_numOutputChannels);
        }
    }
}

void SoundDeviceNetwork::workerWriteSilence(NetworkOutputStreamWorkerPtr pWorker, int frames) {
    if (!pWorker->threadWaiting()) {
        pWorker->addFramesWritten(frames);
        return;
    }

    QSharedPointer<FIFO<CSAMPLE>> pFifo = pWorker->getOutputFifo();
    if (pFifo) {
        int writeAvailable = pFifo->writeAvailable();
        int writeRequired = frames * m_numOutputChannels;
        if (writeAvailable < writeRequired) {
            kLogger.warning() << "writeSilence: worker buffer full, losing samples";
            pWorker->incOverflowCount();
        }

        int clearCount = math_min(writeAvailable, writeRequired);
        if (clearCount > 0) {
            CSAMPLE* dataPtr1;
            ring_buffer_size_t size1;
            CSAMPLE* dataPtr2;
            ring_buffer_size_t size2;

            (void)pFifo->aquireWriteRegions(clearCount,
                    &dataPtr1, &size1, &dataPtr2, &size2);
            SampleUtil::clear(dataPtr1, size1);
            if (size2 > 0) {
                SampleUtil::clear(dataPtr2, size2);
            }
            pFifo->releaseWriteRegions(clearCount);

            // we advance the frame only by the samples we have actually cleared
            pWorker->addFramesWritten(clearCount / m_numOutputChannels);
        }
    }
}

void SoundDeviceNetwork::callbackProcessClkRef() {
    const SINT framesPerBuffer = m_configFramesPerBuffer;

    // This must be the very first call, to measure an exact value
    // NOTE: For network streams the buffer size is always the configured buffer
    //       size
    updateCallbackEntryToDacTime(framesPerBuffer);

    Trace trace("SoundDeviceNetwork::callbackProcessClkRef %1",
                m_deviceId.name);


    if (!m_denormals) {
        m_denormals = true;

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
#endif

#if defined(__aarch64__)
        // Flush-to-zero on aarch64 is controlled by the Floating-point Control Register
        // Load the register into our variable.
        int64_t savedFPCR;
        asm volatile("mrs %[savedFPCR], FPCR"
                     : [ savedFPCR ] "=r"(savedFPCR));

        qDebug() << "aarch64 FPCR: setting bit 24 to 1 to enable Flush-to-zero";
        // Bit 24 is the flush-to-zero mode control bit. Setting it to 1 flushes denormals to 0.
        asm volatile("msr FPCR, %[src]"
                     :
                     : [ src ] "r"(savedFPCR | (1 << 24)));
#endif
        // verify if flush to zero or denormals to zero works
        // test passes if one of the two flag is set.
        volatile double doubleMin = DBL_MIN; // the smallest normalized double
        VERIFY_OR_DEBUG_ASSERT(doubleMin / 2 == 0.0) {
            qWarning() << "Network Sound: Denormals to zero mode is not working. "
                          "EQs and effects may suffer high CPU load";
        }
        else {
            qDebug() << "Network Sound: Denormals to zero mode is working";
        }
    }

    m_pSoundManager->readProcess(framesPerBuffer);

    {
        ScopedTimer t(QStringLiteral("SoundDeviceNetwork::callbackProcess prepare %1"),
                m_deviceId.name);
        m_pSoundManager->onDeviceOutputCallback(framesPerBuffer);
    }

    m_pSoundManager->writeProcess(framesPerBuffer);

    m_pSoundManager->processUnderflowHappened(framesPerBuffer);

    updateAudioLatencyUsage(framesPerBuffer);
}

void SoundDeviceNetwork::updateCallbackEntryToDacTime(SINT framesPerBuffer) {
    m_clkRefTimer.start();
    qint64 currentTime = m_pNetworkStream->getInputStreamTimeUs();
    // This deadline for the next buffer in microseconds since the Unix epoch
    m_targetTime += static_cast<qint64>(framesPerBuffer / m_sampleRate.toDouble() * 1000000);
    double callbackEntrytoDacSecs = (m_targetTime - currentTime) / 1000000.0;
    callbackEntrytoDacSecs = math_max(callbackEntrytoDacSecs, 0.0001);
    VisualPlayPosition::setCallbackEntryToDacSecs(callbackEntrytoDacSecs, m_clkRefTimer);
    //qDebug() << callbackEntrytoDacSecs << timeSinceLastCbSecs;
}

void SoundDeviceNetwork::updateAudioLatencyUsage(SINT framesPerBuffer) {
    m_framesSinceAudioLatencyUsageUpdate += framesPerBuffer;
    if (m_framesSinceAudioLatencyUsageUpdate > (m_sampleRate.toDouble() / CPU_USAGE_UPDATE_RATE)) {
        double secInAudioCb = m_timeInAudioCallback.toDoubleSeconds();
        m_audioLatencyUsage.set(secInAudioCb /
                (m_framesSinceAudioLatencyUsageUpdate / m_sampleRate.toDouble()));
        m_timeInAudioCallback = mixxx::Duration::empty();
        m_framesSinceAudioLatencyUsageUpdate = 0;
        // qDebug() << m_audioLatencyUsage->get();
    }

    qint64 currentTime = m_pNetworkStream->getInputStreamTimeUs();
    unsigned long sleepUs = 0;
    if (currentTime > m_targetTime) {
        m_pSoundManager->underflowHappened(22);
        //qDebug() << "underflow" << currentTime << m_targetTime;
        m_targetTime = currentTime;
    } else {
        sleepUs = m_targetTime - currentTime;
    }

    //qDebug() << "sleep" << sleepUs;

    // measure time in Audio callback at the very last
    m_timeInAudioCallback += m_clkRefTimer.elapsed();

    // now go to sleep until the next callback
    if (sleepUs > 0) {
        m_pThread->usleep_(sleepUs);
    }
}
