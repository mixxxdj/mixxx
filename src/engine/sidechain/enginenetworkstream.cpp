#include "engine/sidechain/enginenetworkstream.h"

#ifdef __WINDOWS__
#include <windows.h>
#include "util/performancetimer.h"
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#ifdef __WINDOWS__
// For GetSystemTimeAsFileTime and GetSystemTimePreciseAsFileTime
typedef VOID (WINAPI *PgGetSystemTimeFn)(LPFILETIME);
static PgGetSystemTimeFn s_pfpgGetSystemTimeFn = NULL;
#endif

#include "util/sample.h"

const int kNetworkLatencyFrames = 8192; // 185 ms @ 44100 Hz
// Related chunk sizes:
// Mp3 frames = 1152 samples
// Ogg frames = 64 to 8192 samples.
// In Mixxx 1.11 we transmit every decoder-frames at once,
// Which results in case of ogg in a dynamic latency from 0.14 ms to to 185 ms
// Now we have switched to a fixed latency of 8192 frames (stereo samples) =
// which is 185 @ 44100 ms and twice the maximum of the max mixxx audio buffer
const int kBufferFrames = kNetworkLatencyFrames * 4; // 743 ms @ 44100 Hz
// normally * 2 is sufficient.
// We allow to buffer two extra chunks for a CPU overload case, when
// the broadcast thread is not scheduled in time.

EngineNetworkStream::EngineNetworkStream(int numOutputChannels,
                                         int numInputChannels)
    : m_pOutputFifo(NULL),
      m_pInputFifo(NULL),
      m_numOutputChannels(numOutputChannels),
      m_numInputChannels(numInputChannels),
      m_sampleRate(0),
      m_streamStartTimeUs(-1),
      m_streamFramesWritten(0),
      m_streamFramesRead(0),
      m_writeOverflowCount(0) {
    if (numOutputChannels) {
        m_pOutputFifo = new FIFO<CSAMPLE>(numOutputChannels * kBufferFrames);
    }
    if (numInputChannels) {
        m_pInputFifo = new FIFO<CSAMPLE>(numInputChannels * kBufferFrames);
    }

#ifdef __WINDOWS__
    // Resolution:
    // 15   ms for GetSystemTimeAsFileTime
    //  0.4 ms for GetSystemTimePreciseAsFileTime
    // Performance:
    //    9 cycles for GetSystemTimeAsFileTime
    // 2761 cycles for GetSystemTimePreciseAsFileTime
    HMODULE kernel32_dll = LoadLibraryW(L"kernel32.dll");
    if (kernel32_dll) {
        // for a 0.0004 ms Resolution on Win8
        s_pfpgGetSystemTimeFn = (PgGetSystemTimeFn)GetProcAddress(
                kernel32_dll, "GetSystemTimePreciseAsFileTime");
    }
#endif
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
    if (m_pWorker.isNull()) {
        return;
    }

    //qDebug() << "EngineNetworkStream::write()" << frames;
    if (!m_pWorker->threadWaiting()) {
        // no thread waiting, so we can advance the stream without
        // buffering
        m_streamFramesWritten += frames;
        return;
    }
    int writeAvailable = m_pOutputFifo->writeAvailable();
    int writeRequired = frames * m_numOutputChannels;
    if (writeAvailable < writeRequired) {
        qDebug() << "EngineNetworkStream::write() buffer full, loosing samples";
        NetworkStreamWorker::debugState();
        m_writeOverflowCount++;
    }
    int copyCount = math_min(writeAvailable, writeRequired);
    if (copyCount > 0) {
        (void)m_pOutputFifo->write(buffer, copyCount);
        // we advance the frame only by the samples we have actually copied
        // This means in case of buffer full (where we loose some frames)
        // we do not get out of sync, and the syncing code tries to catch up the
        // stream by writing silence, once the buffer is free.
        m_streamFramesWritten += copyCount / m_numOutputChannels;
    }
    scheduleWorker();
}

void EngineNetworkStream::writeSilence(int frames) {
    if (m_pWorker.isNull()) {
        return;
    }
    //qDebug() << "EngineNetworkStream::writeSilence()" << frames;
    if (!m_pWorker->threadWaiting()) {
        // no thread waiting, so we can advance the stream without
        // buffering
        m_streamFramesWritten += frames;
        return;
    }
    int writeAvailable = m_pOutputFifo->writeAvailable();
    int writeRequired = frames * m_numOutputChannels;
    if (writeAvailable < writeRequired) {
        qDebug() << "EngineNetworkStream::writeSilence() buffer full";
        NetworkStreamWorker::debugState();
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

        // we advance the frame only by the samples we have actually cleared
        m_streamFramesWritten += clearCount / m_numOutputChannels;
    }
    scheduleWorker();
}

void EngineNetworkStream::scheduleWorker() {
    if (m_pWorker.isNull()) {
        return;
    }
    if (m_pOutputFifo->readAvailable()
            >= m_numOutputChannels * kNetworkLatencyFrames) {
        m_pWorker->outputAvailable();
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
    // Instead of ms resolution we use a us resolution to allow low latency settings
    // will overflow > 200,000 years
#ifdef __WINDOWS__
    FILETIME ft;
    qint64 t;
    // no GetSystemTimePreciseAsFileTime available, fall
    // back to GetSystemTimeAsFileTime. This happens before
    // Windows 8 and Windows Server 2012
    // GetSystemTime?AsFileTime is NTP adjusted
    // QueryPerformanceCounter depends on the CPU crystal
    if(s_pfpgGetSystemTimeFn) {
        s_pfpgGetSystemTimeFn(&ft);
        return ((qint64)ft.dwHighDateTime << 32 | ft.dwLowDateTime) / 10;
    } else {
        static qint64 oldNow = 0;
        static qint64 incCount = 0;
        static PerformanceTimer timerSinceInc;
        GetSystemTimeAsFileTime(&ft);
        qint64 now = ((qint64)ft.dwHighDateTime << 32 | ft.dwLowDateTime) / 10;
        if (now == oldNow) {
            // timer was not incremented since last call (< 15 ms)
            // Add time since last function call after last increment
            // This reduces the jitter < one call cycle which is sufficient
            LARGE_INTEGER li;
            now += timerSinceInc.elapsed().toIntegerMicros();
        } else {
            // timer was incremented
            LARGE_INTEGER li;
            timerSinceInc.start();
            oldNow = now;
        }
        return now;
    }
#elif defined(__APPLE__)
    // clock_gettime is not implemented on OSX
    // gettimeofday can go backward due to NTP adjusting
    // this will work here, because we take the stream start time for reference
    struct timeval mtv;
    gettimeofday(&mtv, NULL);
    return (qint64)(mtv.tv_sec) * 1000000 + mtv.tv_usec;
#else
    // CLOCK_MONOTONIC is NTP adjusted
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
#endif
}

void EngineNetworkStream::addWorker(QSharedPointer<NetworkStreamWorker> pWorker) {
    m_pWorker = pWorker;
    if (m_pWorker) {
        m_pWorker->setOutputFifo(m_pOutputFifo);
    }
}
