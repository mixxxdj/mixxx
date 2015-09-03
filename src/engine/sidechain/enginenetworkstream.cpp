#ifdef __WINDOWS__
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif


#include "engine/sidechain/enginenetworkstream.h"

#include "sampleutil.h"

const int kBufferFrames = 32768; // 743 ms @ 44100 Hz
const int kNetworkLatencyFrames = 8192; // 743 ms @ 44100 Hz
// Related chunk sizes:
// Mp3 frames = 1152 samples
// Ogg frames = 64 to 8192 samples.
// In Mixxx 1.11 we transmit every decoder-frames at once,
// Which results in case of ogg in a dynamic latency from 0.14 ms to to 180 ms
// Now we have switched to a fixed latency of 8192 frames (stereo samples) =
// which is 185 @ 44100 ms and twice the maximum of the max mixxx audio buffer

EngineNetworkStream::EngineNetworkStream(int numOutputChannels,
                                         int numInputChannels)
    : m_pOutputFifo(NULL),
      m_pInputFifo(NULL),
      m_numOutputChannels(numOutputChannels),
      m_numInputChannels(numInputChannels),
      m_sampleRate(0),
      m_streamStartTimeUs(-1),
      m_streamFramesWritten(0),
      m_streamFramesRead(0) {
    if (numOutputChannels) {
        m_pOutputFifo = new FIFO<CSAMPLE>(numOutputChannels * kBufferFrames);
    }
    if (numInputChannels) {
        m_pInputFifo = new FIFO<CSAMPLE>(numInputChannels * kBufferFrames);
    }
}

EngineNetworkStream::~EngineNetworkStream() {
    if (m_streamStartTimeUs >= 0) {
        stopStream();
    }
    delete m_pOutputFifo;
    delete m_pInputFifo;
}

void EngineNetworkStream::startStream(double sampleRate) {
    m_sampleRate = sampleRate;
    m_streamStartTimeUs = getNetworkTimeUs();
    m_streamFramesWritten = 0;
}

void EngineNetworkStream::stopStream() {
    m_streamStartTimeUs = -1;
}

int EngineNetworkStream::getWriteExpected() {
    return static_cast<int>(getStreamTimeFrames() - m_streamFramesWritten);
}

int EngineNetworkStream::getReadExpected() {
    return static_cast<int>(getStreamTimeFrames() - m_streamFramesRead);
}

void EngineNetworkStream::write(const CSAMPLE* buffer, int frames) {
    m_streamFramesWritten += frames;
    //qDebug() << "EngineNetworkStream::write()" << frames;
    if (!m_pWorker->threadWaiting()) return;
    int writeAvailable = m_pOutputFifo->writeAvailable();
    int writeRequired = frames * m_numOutputChannels;
    if (writeAvailable < writeRequired) {
        qDebug() << "EngineNetworkStream::write() buffer full";
    }
    int copyCount = math_min(writeAvailable, writeRequired);
    if (copyCount > 0) {
        (void)m_pOutputFifo->write(buffer, copyCount);
    }
    scheduleWorker();
}

void EngineNetworkStream::writeSilence(int frames) {
    m_streamFramesWritten += frames;
    //qDebug() << "EngineNetworkStream::writeSilence()" << frames;
    if (!m_pWorker->threadWaiting()) return;
    int writeAvailable = m_pOutputFifo->writeAvailable();
    int writeRequired = frames * m_numOutputChannels;
    if (writeAvailable < writeRequired) {
        qDebug() << "EngineNetworkStream::write() buffer full";
    }
    int clearCount = math_min(writeAvailable, writeRequired);
    if (clearCount > 0) {
        CSAMPLE* dataPtr1;
        ring_buffer_size_t size1;
        CSAMPLE* dataPtr2;
        ring_buffer_size_t size2;
        (void)m_pOutputFifo->aquireWriteRegions(clearCount,
                &dataPtr1, &size1, &dataPtr2, &size2);
        SampleUtil::clear(dataPtr1,size1);
        if (size2 > 0) {
            SampleUtil::clear(dataPtr2,size2);
        }
        m_pOutputFifo->releaseWriteRegions(clearCount);
    }
    scheduleWorker();
}

void EngineNetworkStream::scheduleWorker() {
    if (m_pOutputFifo->readAvailable()
            >= m_numOutputChannels * kNetworkLatencyFrames) {
        m_pWorker->outputAvailabe(m_pOutputFifo);
    }
}

void EngineNetworkStream::read(CSAMPLE* buffer, int frames) {
    int readAvailable = m_pOutputFifo->readAvailable();
    int readRequired = frames * m_numInputChannels;
    int copyCount = math_min(readAvailable, readRequired);
    if (copyCount > 0) {
        (void)m_pOutputFifo->read(buffer, copyCount);
        buffer += copyCount;
    }
    if (readAvailable < readRequired) {
        // Fill missing Samples with silence
        int silenceCount = readRequired - readAvailable;
        qDebug() << "EngineNetworkStream::write flushed" << readRequired
                 << "samples";
        SampleUtil::clear(buffer, silenceCount);
    }
}

qint64 EngineNetworkStream::getStreamTimeFrames() {
    return static_cast<double>(getStreamTimeUs()) * m_sampleRate / 1000000.0;
}

qint64 EngineNetworkStream::getStreamTimeUs() {
    return getNetworkTimeUs() - m_streamStartTimeUs;
}

// static
qint64 EngineNetworkStream::getNetworkTimeUs() {
    // This matches the GPL2 implementation found in
    // https://github.com/codders/libshout/blob/a17fb84671d3732317b0353d7281cc47e2df6cf6/src/timing/timing.c
    // Instead of ms resuolution we use a us resolution to allow low latency settings
    // will overflow > 200,000 years
#ifdef __WINDOWS__
    FILETIME ft;
    int64_t t;
    GetSystemTimeAsFileTime(&ft);
    return ((qint64)ft.dwHighDateTime << 32 | ft.dwLowDateTime) / 10;
#else
    struct timeval mtv;
    gettimeofday(&mtv, NULL);
    return (qint64)(mtv.tv_sec) * 1000000 + mtv.tv_usec;
#endif
}

void EngineNetworkStream::addWorker(QSharedPointer<SideChainWorker> pWorker) {
    m_pWorker = pWorker;
}
