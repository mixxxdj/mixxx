#include "soundio/sounddevicenetwork.h"

#include <QtDebug>

#include "waveform/visualplayposition.h"
#include "util/timer.h"
#include "util/trace.h"
#include "controlobjectslave.h"
#include "controlobject.h"
#include "util/denormalsarezero.h"
#include "engine/sidechain/enginenetworkstream.h"
#include "float.h"
#include "soundio/sounddevice.h"
#include "soundio/soundmanager.h"
#include "soundio/soundmanagerutil.h"
#include "util/sample.h"

// static
volatile int SoundDeviceNetwork::m_underflowHappened = 0;

SoundDeviceNetwork::SoundDeviceNetwork(UserSettingsPointer config,
                                       SoundManager *sm,
                                       QSharedPointer<EngineNetworkStream> pNetworkStream)
        : SoundDevice(config, sm),
          m_pNetworkStream(pNetworkStream),
          m_outputFifo(NULL),
          m_inputFifo(NULL),
          m_outputDrift(false),
          m_inputDrift(false),
          m_underflowUpdateCount(0),
          m_framesSinceAudioLatencyUsageUpdate(0),
          m_pThread(NULL),
          m_denormals(false),
          m_targetTime(0),
          m_lastCallbackEntrytoDacSecs(0) {
    // Setting parent class members:
    m_hostAPI = "Network stream";
    m_dSampleRate = 44100.0;
    m_strInternalName = kNetworkDeviceInternalName;
    m_strDisplayName = QObject::tr("Network stream");
    m_iNumInputChannels = pNetworkStream->getNumInputChannels();
    m_iNumOutputChannels = pNetworkStream->getNumOutputChannels();

    m_pMasterAudioLatencyOverloadCount = new ControlObjectSlave("[Master]",
            "audio_latency_overload_count");
    m_pMasterAudioLatencyUsage = new ControlObjectSlave("[Master]",
            "audio_latency_usage");
    m_pMasterAudioLatencyOverload = new ControlObjectSlave("[Master]",
            "audio_latency_overload");
}

SoundDeviceNetwork::~SoundDeviceNetwork() {
    delete m_pMasterAudioLatencyOverloadCount;
    delete m_pMasterAudioLatencyUsage;
    delete m_pMasterAudioLatencyOverload;
}

Result SoundDeviceNetwork::open(bool isClkRefDevice, int syncBuffers) {
    Q_UNUSED(syncBuffers);
    qDebug() << "SoundDeviceNetwork::open()" << getInternalName();

    // Sample rate
    if (m_dSampleRate <= 0) {
        m_dSampleRate = 44100.0;
    }

    // Get latency in milleseconds
    qDebug() << "framesPerBuffer:" << m_framesPerBuffer;

    m_audioBufferTime = mixxx::Duration::fromSeconds(
            m_framesPerBuffer / m_dSampleRate);
    qDebug() << "Requested sample rate: " << m_dSampleRate << "Hz, latency:"
             << m_audioBufferTime;

    // Feet the network device buffer directly from the
    // clock reference device callback
    // This is what should work best.
    if (m_iNumOutputChannels) {
        m_outputFifo = new FIFO<CSAMPLE>(
                m_iNumOutputChannels * m_framesPerBuffer * 2);
    }
    if (m_iNumInputChannels) {
        m_inputFifo = new FIFO<CSAMPLE>(
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

        if (m_pMasterAudioLatencyOverloadCount) {
            m_pMasterAudioLatencyOverloadCount->set(0);
        }

        m_targetTime = 0;

        m_pThread = new SoundDeviceNetworkThread(this);
        m_pThread->start(QThread::TimeCriticalPriority);
    }

    return OK;
}

bool SoundDeviceNetwork::isOpen() const {
    return (m_inputFifo != NULL || m_outputFifo != NULL);
}

Result SoundDeviceNetwork::close() {
    //qDebug() << "SoundDeviceNetwork::close()" << getInternalName();
    m_pNetworkStream->stopStream();
    if (m_outputFifo) {
        delete m_outputFifo;
        m_outputFifo = NULL;
    }
    if (m_inputFifo) {
        delete m_inputFifo;
        m_inputFifo = NULL;
    }
    return OK;
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
                //qDebug() << "SoundDevicePortAudio::readProcess() skip one frame"
                //        << (float)writeAvailable / inChunkSize << (float)readAvailable / inChunkSize;
                m_pNetworkStream->read(dataPtr1, 1);
            } else {
                m_inputDrift = true;
            }
        } else if (readAvailable < inChunkSize / 2) {
            // We should read at least inChunkSize
            if (m_inputDrift) {
                // duplicate one frame
                //qDebug() << "SoundDevicePortAudio::readProcess() duplicate one frame"
                //        << (float)writeAvailable / inChunkSize << (float)readAvailable / inChunkSize;
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
        m_underflowHappened = 1;
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
        m_underflowHappened = 1;
        //qDebug() << "writeProcess():" << (float) writeAvailable / outChunkSize << "Overflow";
    }
    //qDebug() << "writeProcess():" << (float) writeAvailable / outChunkSize;
    if (writeCount) {
        CSAMPLE* dataPtr1;
        ring_buffer_size_t size1;
        CSAMPLE* dataPtr2;
        ring_buffer_size_t size2;
        // We use size1 and size2, so we can ignore the return value
        (void)m_outputFifo->aquireWriteRegions(writeCount, &dataPtr1,
                &size1, &dataPtr2, &size2);
        // Fetch fresh samples and write to the the output buffer
        composeOutputBuffer(dataPtr1, size1 / m_iNumOutputChannels, 0,
                static_cast<unsigned int>(m_iNumOutputChannels));
        if (size2 > 0) {
            composeOutputBuffer(dataPtr2,
                    size2 / m_iNumOutputChannels,
                    size1 / m_iNumOutputChannels,
                    static_cast<unsigned int>(m_iNumOutputChannels));
        }
        m_outputFifo->releaseWriteRegions(writeCount);
    }
    writeAvailable = m_pNetworkStream->getWriteExpected()
            * m_iNumOutputChannels;
    int readAvailable = m_outputFifo->readAvailable();
    int copyCount = qMin(readAvailable, writeAvailable);
    //qDebug() << "SoundDevicePortAudio::writeProcess()" << toRead << writeAvailable;
    if (copyCount > 0) {
        CSAMPLE* dataPtr1;
        ring_buffer_size_t size1;
        CSAMPLE* dataPtr2;
        ring_buffer_size_t size2;
        m_outputFifo->aquireReadRegions(copyCount,
                &dataPtr1, &size1, &dataPtr2, &size2);
        if (writeAvailable >= outChunkSize * 2) {
            // Underflow
            //qDebug() << "SoundDeviceNetwork::writeProcess() Buffer empty";
            // catch up by filling buffer until we are synced
            m_pNetworkStream->writeSilence(writeAvailable - copyCount);
            m_underflowHappened = 1;
        } else if (writeAvailable > readAvailable + outChunkSize / 2) {
            // try to keep PAs buffer filled up to 0.5 chunks
            if (m_outputDrift) {
                // duplicate one frame
                //qDebug() << "SoundDeviceNetwork::writeProcess() duplicate one frame"
                //         << (float)writeAvailable / outChunkSize << (float)readAvailable / outChunkSize;
                m_pNetworkStream->write(dataPtr1, 1);
            } else {
                m_outputDrift = true;
            }
        } else if (writeAvailable < outChunkSize / 2) {
            // We are not able to store all new frames
            if (m_outputDrift) {
                //qDebug() << "SoundDeviceNetwork::writeProcess() skip one frame"
                //         << (float)writeAvailable / outChunkSize << (float)readAvailable / outChunkSize;
                ++copyCount;
            } else {
                m_outputDrift = true;
            }
        } else {
            m_outputDrift = false;
        }

        m_pNetworkStream->write(dataPtr1,
                size1 / m_iNumOutputChannels);
        if (size2 > 0) {
            m_pNetworkStream->write(dataPtr2,
                    size2 / m_iNumOutputChannels);
        }
        m_outputFifo->releaseReadRegions(copyCount);
    }
}

void SoundDeviceNetwork::callbackProcessClkRef() {
    // This must be the very first call, to measure an exact value
    updateCallbackEntryToDacTime();

    Trace trace("SoundDeviceNetwork::callbackProcessClkRef %1",
                getInternalName());


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
        DEBUG_ASSERT_AND_HANDLE(doubleMin / 2 == 0.0) {
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
                getInternalName());
        m_pSoundManager->onDeviceOutputCallback(m_framesPerBuffer);
    }

    m_pSoundManager->writeProcess();

    if (m_underflowUpdateCount == 0) {
        if (m_underflowHappened) {
            m_pMasterAudioLatencyOverload->set(1.0);
            m_pMasterAudioLatencyOverloadCount->set(
                    m_pMasterAudioLatencyOverloadCount->get() + 1);
            m_underflowUpdateCount = CPU_OVERLOAD_DURATION * m_dSampleRate
                    / m_framesPerBuffer / 1000;
            m_underflowHappened = 0; // reseting her is not thread save,
                                     // but that is OK, because we count only
                                     // 1 underflow each 500 ms
        } else {
            m_pMasterAudioLatencyOverload->set(0.0);
        }
    } else {
        --m_underflowUpdateCount;
    }

    updateAudioLatencyUsage();
}

void SoundDeviceNetwork::updateCallbackEntryToDacTime() {
    m_clkRefTimer.start();
    qint64 currentTime = m_pNetworkStream->getStreamTimeUs();
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
        m_timeInAudioCallback.reset();
        m_framesSinceAudioLatencyUsageUpdate = 0;
        //qDebug() << m_pMasterAudioLatencyUsage->get();
    }

    qint64 currentTime = m_pNetworkStream->getStreamTimeUs();
    unsigned long sleepUs = 0;
    if (currentTime > m_targetTime) {
        m_underflowHappened = true;
        m_targetTime = currentTime;
        qDebug() << "underflow" << currentTime << m_targetTime;
    } else {
        sleepUs = m_targetTime - currentTime;
    }

    // measure time in Audio callback at the very last
    m_timeInAudioCallback += m_clkRefTimer.elapsed();

    // now go to sleep until the next callback
    if (sleepUs > 0) {
        m_pThread->usleep_(sleepUs);
    }
}
