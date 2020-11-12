#include "soundio/sounddevicenetwork.h"

#include <QtDebug>

#include "waveform/visualplayposition.h"
#include "util/timer.h"
#include "util/trace.h"
#include "control/controlproxy.h"
#include "control/controlobject.h"
#include "util/denormalsarezero.h"
#include "engine/sidechain/enginenetworkstream.h"
#include "float.h"
#include "soundio/sounddevice.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"
#include "util/logger.h"
#include "util/sample.h"

namespace {
const int kNetworkLatencyFrames = 8192; // 185 ms @ 44100 Hz
// Related chunk sizes:
// Mp3 frames = 1152 samples
// Ogg frames = 64 to 8192 samples.
// In Mixxx 1.11 we transmit every decoder-frames at once,
// Which results in case of ogg in a dynamic latency from 0.14 ms to to 185 ms
// Now we have switched to a fixed latency of 8192 frames (stereo samples) =
// which is 185 @ 44100 ms and twice the maximum of the max mixxx audio buffer

const mixxx::Logger kLogger("SoundDeviceNetwork");
}

SoundDeviceNetwork::SoundDeviceNetwork(UserSettingsPointer config,
                                       SoundManager *sm,
                                       QSharedPointer<EngineNetworkStream> pNetworkStream)
        : SoundDevice(config, sm),
          m_pNetworkStream(pNetworkStream),
          m_inputDrift(false),
          m_framesSinceAudioLatencyUsageUpdate(0),
          m_denormals(false),
          m_targetTime(0) {
    // Setting parent class members:
    m_hostAPI = "Network stream";
    m_dSampleRate = 44100.0;
    m_deviceId.name = kNetworkDeviceInternalName;
    m_strDisplayName = QObject::tr("Network stream");
    m_iNumInputChannels = pNetworkStream->getNumInputChannels();
    m_iNumOutputChannels = pNetworkStream->getNumOutputChannels();

    m_pMasterAudioLatencyUsage = std::make_unique<ControlProxy>("[Master]",
            "audio_latency_usage");
}

SoundDeviceNetwork::~SoundDeviceNetwork() {
}

SoundDeviceError SoundDeviceNetwork::open(bool isClkRefDevice, int syncBuffers) {
    Q_UNUSED(syncBuffers);
    kLogger.debug() << "open:" << m_deviceId.name;

    // Sample rate
    if (m_dSampleRate <= 0) {
        m_dSampleRate = 44100.0;
    }

    qDebug() << "framesPerBuffer:" << m_framesPerBuffer;

    m_audioBufferTime = mixxx::Duration::fromSeconds(
            m_framesPerBuffer / m_dSampleRate);
    qDebug() << "Requested sample rate: " << m_dSampleRate << "Hz, latency:"
             << m_audioBufferTime;

    // Feed the network device buffer directly from the
    // clock reference device callback
    // This is what should work best.
    if (m_iNumOutputChannels) {
        m_outputFifo = std::make_unique<FIFO<CSAMPLE> >(
                m_iNumOutputChannels * m_framesPerBuffer * 2);
    }
    if (m_iNumInputChannels) {
        m_inputFifo = std::make_unique<FIFO<CSAMPLE> >(
                m_iNumInputChannels * m_framesPerBuffer * 2);
    }

    m_pNetworkStream->startStream(m_dSampleRate);

    // Create the callback Thread if requested
    if (isClkRefDevice) {
        // Update the samplerate and latency ControlObjects, which allow the
        // waveform view to properly correct for the latency.
        ControlObject::set(ConfigKey("[Master]", "latency"),
                m_audioBufferTime.toDoubleMillis());
        ControlObject::set(ConfigKey("[Master]", "samplerate"), m_dSampleRate);
        ControlObject::set(ConfigKey("[Master]", "audio_buffer_size"),
                m_audioBufferTime.toDoubleMillis());

        // Network stream was just started above so we have to wait until
        // we can pass one chunk.
        // The first callback runs early to do the one time setups
        m_targetTime = m_audioBufferTime.toIntegerMicros();

        m_pThread = std::make_unique<SoundDeviceNetworkThread>(this);
        m_pThread->start(QThread::TimeCriticalPriority);
    }

    return SOUNDDEVICE_ERROR_OK;
}

bool SoundDeviceNetwork::isOpen() const {
    return (m_inputFifo != NULL || m_outputFifo != NULL);
}

SoundDeviceError SoundDeviceNetwork::close() {
    //kLogger.debug() << "close:" << getInternalName();
    m_pNetworkStream->stopStream();
    if (m_pThread) {
        m_pThread->stop();
        m_pThread->wait();
        m_pThread.reset();
    }

    m_outputFifo.reset();
    m_inputFifo.reset();

    return SOUNDDEVICE_ERROR_OK;
}

QString SoundDeviceNetwork::getError() const {
    return QString();
}

void SoundDeviceNetwork::readProcess() {
    if (!m_inputFifo || !m_pNetworkStream || !m_iNumInputChannels) return;

    int inChunkSize = m_framesPerBuffer * m_iNumInputChannels;
    int readAvailable = m_pNetworkStream->getReadExpected()
            * m_iNumInputChannels;
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
                size1 / m_iNumInputChannels);
        CSAMPLE* lastFrame = &dataPtr1[size1 - m_iNumInputChannels];
        if (size2 > 0) {
            m_pNetworkStream->read(dataPtr2,
                    size2 / m_iNumInputChannels);
            lastFrame = &dataPtr2[size2 - m_iNumInputChannels];
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
                (void) m_inputFifo->aquireWriteRegions(
                        m_iNumInputChannels, &dataPtr1, &size1,
                        &dataPtr2, &size2);
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
                size1 / m_iNumInputChannels, 0,
                m_iNumInputChannels);
        if (size2 > 0) {
            composeInputBuffer(dataPtr2,
                    size2 / m_iNumInputChannels,
                    size1 / m_iNumInputChannels,
                    m_iNumInputChannels);
        }
        m_inputFifo->releaseReadRegions(readCount);
    }
    if (readCount < inChunkSize) {
        // Fill remaining buffers with zeros
        clearInputBuffer(inChunkSize - readCount, readCount);
    }

    m_pSoundManager->pushInputBuffers(m_audioInputs, m_framesPerBuffer);
}

void SoundDeviceNetwork::writeProcess() {
    if (!m_outputFifo || !m_pNetworkStream) return;

    int outChunkSize = m_framesPerBuffer * m_iNumOutputChannels;
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
        composeOutputBuffer(dataPtr1, size1 / m_iNumOutputChannels, 0, m_iNumOutputChannels);
        if (size2 > 0) {
            composeOutputBuffer(dataPtr2,
                    size2 / m_iNumOutputChannels,
                    size1 / m_iNumOutputChannels,
                    m_iNumOutputChannels);
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

    QVector<NetworkOutputStreamWorkerPtr> workers =
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
    int writeExpected = static_cast<int>(pWorker->getStreamTimeFrames() - pWorker->framesWritten());

    int writeAvailable = writeExpected * m_iNumOutputChannels;
    int copyCount = qMin(readAvailable, writeAvailable);

    if (copyCount > 0) {
        if (writeAvailable - copyCount > outChunkSize) {
            // Underflow
            //kLogger.debug() << "workerWriteProcess: buffer empty";
            // catch up by filling buffer until we are synced
            workerWriteSilence(pWorker, writeAvailable - copyCount);
            m_pSoundManager->underflowHappened(24);
        } else if (writeAvailable - copyCount > outChunkSize / 2) {
            // try to keep PAs buffer filled up to 0.5 chunks
            if (pWorker->outputDrift()) {
                // duplicate one frame
                //kLogger.debug() << "workerWriteProcess() duplicate one frame"
                //                << (float)writeAvailable / outChunkSize << (float)readAvailable / outChunkSize;
                workerWrite(pWorker, dataPtr1, 1);
            } else {
                pWorker->setOutputDrift(true);
            }
        } else if (writeAvailable < outChunkSize / 2 ||
                readAvailable > outChunkSize * 1.5
           ) {
            // We are not able to store at least the half of the new frames
            // or we have a risk of an m_outputFifo overflow
            if (pWorker->outputDrift()) {
                //kLogger.debug() << "SoundDeviceNetwork::workerWriteProcess() skip one frame"
                //                << (float)writeAvailable / outChunkSize << (float)readAvailable / outChunkSize;
                copyCount = qMin(readAvailable, copyCount + m_iNumOutputChannels);
            } else {
                pWorker->setOutputDrift(true);
            }
        } else {
            pWorker->setOutputDrift(false);
        }

        workerWrite(pWorker, dataPtr1, size1 / m_iNumOutputChannels);
        if (size2 > 0) {
            workerWrite(pWorker, dataPtr2, size2 / m_iNumOutputChannels);
        }

        QSharedPointer<FIFO<CSAMPLE>> pFifo = pWorker->getOutputFifo();
        if (pFifo) {
            // interval = copyCount
            // Check for desired kNetworkLatencyFrames + 1/2 interval to
            // avoid big jitter due to interferences with sync code
            if (pFifo->readAvailable() + copyCount / 2
                    >= (m_iNumOutputChannels * kNetworkLatencyFrames)) {
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
        int writeRequired = frames * m_iNumOutputChannels;
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
            pWorker->addFramesWritten(copyCount / m_iNumOutputChannels);
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
        int writeRequired = frames * m_iNumOutputChannels;
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
            pWorker->addFramesWritten(clearCount / m_iNumOutputChannels);
        }
    }
}

void SoundDeviceNetwork::callbackProcessClkRef() {
    // This must be the very first call, to measure an exact value
    updateCallbackEntryToDacTime();

    Trace trace("SoundDeviceNetwork::callbackProcessClkRef %1",
                m_deviceId.name);


    if (!m_denormals) {
        m_denormals = true;
        // This disables the denormals calculations, to avoid a
        // performance penalty of ~20
        // https://bugs.launchpad.net/mixxx/+bug/1404401
#ifdef __SSE__
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
        // verify if flush to zero or denormals to zero works
        // test passes if one of the two flag is set.
        volatile double doubleMin = DBL_MIN; // the smallest normalized double
        VERIFY_OR_DEBUG_ASSERT(doubleMin / 2 == 0.0) {
            qWarning() << "SSE: Denormals to zero mode is not working. EQs and effects may suffer high CPU load";
        } else {
            qDebug() << "SSE: Denormals to zero mode is working";
        }
#else
        qWarning() << "No SSE: No denormals to zero mode available. EQs and effects may suffer high CPU load";
#endif
    }

    m_pSoundManager->readProcess();

    {
        ScopedTimer t("SoundDevicePortAudio::callbackProcess prepare %1",
                m_deviceId.name);
        m_pSoundManager->onDeviceOutputCallback(m_framesPerBuffer);
    }

    m_pSoundManager->writeProcess();

    m_pSoundManager->processUnderflowHappened();

    updateAudioLatencyUsage();
}

void SoundDeviceNetwork::updateCallbackEntryToDacTime() {
    m_clkRefTimer.start();
    qint64 currentTime = m_pNetworkStream->getInputStreamTimeUs();
    m_targetTime += m_audioBufferTime.toIntegerMicros();
    double callbackEntrytoDacSecs = (m_targetTime - currentTime) / 1000000.0;
    callbackEntrytoDacSecs = math_max(callbackEntrytoDacSecs, 0.0001);
    VisualPlayPosition::setCallbackEntryToDacSecs(callbackEntrytoDacSecs, m_clkRefTimer);
    //qDebug() << callbackEntrytoDacSecs << timeSinceLastCbSecs;
}

void SoundDeviceNetwork::updateAudioLatencyUsage() {
    m_framesSinceAudioLatencyUsageUpdate += m_framesPerBuffer;
    if (m_framesSinceAudioLatencyUsageUpdate
            > (m_dSampleRate / CPU_USAGE_UPDATE_RATE)) {
        double secInAudioCb = m_timeInAudioCallback.toDoubleSeconds();
        m_pMasterAudioLatencyUsage->set(secInAudioCb /
                (m_framesSinceAudioLatencyUsageUpdate / m_dSampleRate));
        m_timeInAudioCallback = mixxx::Duration::empty();
        m_framesSinceAudioLatencyUsageUpdate = 0;
        //qDebug() << m_pMasterAudioLatencyUsage->get();
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
