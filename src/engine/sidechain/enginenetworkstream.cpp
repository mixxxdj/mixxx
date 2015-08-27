#ifdef __WINDOWS__
#include <windows.h>
#include <mmsystem.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif


#include "engine/sidechain/enginenetworkstream.h"

#include "sampleutil.h"

unsigned int kBufferFrames = 32768; // 743 ms @ 44100 Hz

EngineNetworkStream::EngineNetworkStream(double sampleRate,
                                         int numOutputChannels,
                                         int numInputChannels)
    : m_pOutputFifo(NULL),
      m_pInputFifo(NULL),
      m_numOutputChannels(numOutputChannels),
      m_numInputChannels(numInputChannels),
      m_sampleRate(sampleRate),
      m_streamStartTimeMs(-1),
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
    if (m_streamStartTimeMs >= 0) {
        stopStream();
    }
    delete m_pOutputFifo;
    delete m_pInputFifo;
}

void EngineNetworkStream::startStream() {
    m_streamStartTimeMs = getNetworkTimeMs();
    m_streamFramesWritten = 0;
}

void EngineNetworkStream::stopStream() {
    m_streamStartTimeMs = -1;
}

int EngineNetworkStream::getWriteExpected() {
    return static_cast<int>(getStreamTimeFrames() - m_streamFramesWritten);
}

int EngineNetworkStream::getReadExpected() {
    return static_cast<int>(getStreamTimeFrames() - m_streamFramesRead);
}

void EngineNetworkStream::write(const CSAMPLE* buffer, int frames) {
    int writeAvailable = m_pOutputFifo->writeAvailable();
    int writeRequired = frames * m_numOutputChannels;
    if (writeAvailable < writeRequired) {
        // Flush outdated frames to free space for writing
        int readRequired = writeRequired - writeAvailable;
        qDebug() << "EngineNetworkStream::write flushed" << readRequired
                 << "samples";
        m_pOutputFifo->flushReadData(readRequired);
        writeAvailable = m_pOutputFifo->writeAvailable();
    }
    int copyCount = math_min(writeAvailable, writeRequired);
    if (copyCount > 0) {
        (void)m_pOutputFifo->write(buffer, copyCount);
    }
    m_streamFramesWritten += frames;
}

void EngineNetworkStream::writeSilence(int frames) {
    int writeAvailable = m_pOutputFifo->writeAvailable();
    int writeRequired = frames * m_numOutputChannels;
    if (writeAvailable < writeRequired) {
        // Flush outdated frames to free space for writing
        int readRequired = writeRequired - writeAvailable;
        qDebug() << "EngineNetworkStream::writeSilence flushed" << readRequired
                 << "samples";
        m_pOutputFifo->flushReadData(readRequired);
        writeAvailable = m_pOutputFifo->writeAvailable();
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
    m_streamFramesWritten += frames;
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

qint64 EngineNetworkStream::getStreamTimeMs() {
    return getNetworkTimeMs() - m_streamStartTimeMs;
}

qint64 EngineNetworkStream::getStreamTimeFrames() {
    return static_cast<double>(getStreamTimeMs()) * m_sampleRate / 1000;
}

// static
qint64 EngineNetworkStream::getNetworkTimeMs() {
    // This matches the GPL2 implementation found in
    // https://github.com/codders/libshout/blob/a17fb84671d3732317b0353d7281cc47e2df6cf6/src/timing/timing.c
#ifdef __WINDOWS__
    return timeGetTime();
#else
    struct timeval mtv;
    gettimeofday(&mtv, NULL);
    return (qint64)(mtv.tv_sec) * 1000 + (qint64)(mtv.tv_usec) / 1000;
#endif
}
