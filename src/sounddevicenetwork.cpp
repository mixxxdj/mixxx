
#include <QtDebug>

#include "sounddevicenetwork.h"

#include "soundmanager.h"
#include "sounddevice.h"
#include "soundmanagerutil.h"
#include "controlobject.h"
#include "visualplayposition.h"
#include "util/timer.h"
#include "util/trace.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "sampleutil.h"
#include "controlobjectslave.h"
#include "util/performancetimer.h"
#include "util/denormalsarezero.h"
#include "engine/sidechain/enginenetworkstream.h"

// static
volatile int SoundDeviceNetwork::m_underflowHappend = 0;

SoundDeviceNetwork::SoundDeviceNetwork(ConfigObject<ConfigValue> *config,
                                       SoundManager *sm,
                                       QSharedPointer<EngineNetworkStream> pNetworkStream)
        : SoundDevice(config, sm),
          m_pNetworkStream(pNetworkStream),
          m_outputFifo(NULL),
          m_inputFifo(NULL),
          m_outputDrift(false),
          m_inputDrift(false),
          m_bSetThreadPriority(false),
          m_underflowUpdateCount(0),
          m_nsInAudioCb(0),
          m_framesSinceAudioLatencyUsageUpdate(0) {
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
    double bufferMSec = m_framesPerBuffer / m_dSampleRate * 1000;
    qDebug() << "Requested sample rate: " << m_dSampleRate << "Hz, latency:"
             << bufferMSec << "ms";

    // Create the callback function pointer.
    if (isClkRefDevice) {
        // Network device as clock Reference is not yet supported
        DEBUG_ASSERT(false);
    } else {
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
    }

    m_pNetworkStream->startStream(m_dSampleRate);

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
    m_bSetThreadPriority = false;
    return OK;
}

QString SoundDeviceNetwork::getError() const {
    return m_lastError;
}

void SoundDeviceNetwork::readProcess() {
    if (!m_inputFifo || !m_pNetworkStream) return;

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
        m_underflowHappend = 1;
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
        m_underflowHappend = 1;
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
            m_underflowHappend = 1;
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
