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

#include "broadcast/defs_broadcast.h"
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
const int kBufferFrames = kNetworkLatencyFrames * 4; // 743 ms @ 44100 Hz
// normally * 2 is sufficient.
// We allow to buffer two extra chunks for a CPU overload case, when
// the broadcast thread is not scheduled in time.

const mixxx::Logger kLogger("EngineNetworkStream");
}

EngineNetworkStream::EngineNetworkStream(int numOutputChannels,
                                         int numInputChannels)
    : m_pInputFifo(nullptr),
      m_numOutputChannels(numOutputChannels),
      m_numInputChannels(numInputChannels),
      m_sampleRate(0),
      m_inputStreamStartTimeUs(-1),
      m_inputStreamFramesWritten(0),
      m_inputStreamFramesRead(0),
      m_outputWorkers(BROADCAST_MAX_CONNECTIONS) {
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
    if (m_inputStreamStartTimeUs >= 0) {
        stopStream();
    }

    delete m_pInputFifo;
}

void EngineNetworkStream::startStream(double sampleRate) {
    m_sampleRate = sampleRate;
    m_inputStreamStartTimeUs = getNetworkTimeUs();
    m_inputStreamFramesWritten = 0;

    for (NetworkOutputStreamWorkerPtr worker : qAsConst(m_outputWorkers)) {
        if (worker.isNull()) {
            continue;
        }

        worker->startStream(m_sampleRate, m_numOutputChannels);
    }
}

void EngineNetworkStream::stopStream() {
    m_inputStreamStartTimeUs = -1;

    for (NetworkOutputStreamWorkerPtr worker : qAsConst(m_outputWorkers)) {
        if (worker.isNull()) {
            continue;
        }

        worker->stopStream();
    }
}

int EngineNetworkStream::getReadExpected() {
    return static_cast<int>(getInputStreamTimeFrames() - m_inputStreamFramesRead);
}

void EngineNetworkStream::read(CSAMPLE* buffer, int frames) {
    int readAvailable = m_pInputFifo->readAvailable();
    int readRequired = frames * m_numInputChannels;
    int copyCount = math_min(readAvailable, readRequired);
    if (copyCount > 0) {
        (void)m_pInputFifo->read(buffer, copyCount);
        buffer += copyCount;
    }
    if (readAvailable < readRequired) {
        // Fill missing Samples with silence
        int silenceCount = readRequired - readAvailable;
        kLogger.debug() << "write: flushed"
                        << readRequired << "samples";
        SampleUtil::clear(buffer, silenceCount);
    }
}

qint64 EngineNetworkStream::getInputStreamTimeFrames() {
    return static_cast<qint64>(static_cast<double>(getInputStreamTimeUs()) *
            m_sampleRate / 1000000.0);
}

qint64 EngineNetworkStream::getInputStreamTimeUs() {
    return getNetworkTimeUs() - m_inputStreamStartTimeUs;
}

// static
qint64 EngineNetworkStream::getNetworkTimeUs() {
    // This matches the GPL2 implementation found in
    // https://github.com/codders/libshout/blob/a17fb84671d3732317b0353d7281cc47e2df6cf6/src/timing/timing.c
    // Instead of ms resolution we use a us resolution to allow low latency settings
    // will overflow > 200,000 years
#ifdef __WINDOWS__
    FILETIME ft;
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
            now += timerSinceInc.elapsed().toIntegerMicros();
        } else {
            // timer was incremented
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

void EngineNetworkStream::addOutputWorker(NetworkOutputStreamWorkerPtr pWorker) {
    if (nextOutputSlotAvailable() < 0) {
        kLogger.warning() << "addWorker: can't add worker:"
                          << "no free slot left in internal list";
        return;
    }

    if (pWorker && m_numOutputChannels) {
        int nextNullItem = nextOutputSlotAvailable();
        if(nextNullItem > -1) {
            QSharedPointer<FIFO<CSAMPLE>> workerFifo(
                    new FIFO<CSAMPLE>(m_numOutputChannels * kBufferFrames));
            pWorker->setOutputFifo(workerFifo);
            pWorker->startStream(m_sampleRate, m_numOutputChannels);
            m_outputWorkers[nextNullItem] = pWorker;

            kLogger.debug() << "addWorker: worker added";
            debugOutputSlots();
        }
    }
}

void EngineNetworkStream::removeOutputWorker(NetworkOutputStreamWorkerPtr pWorker) {
    int index = m_outputWorkers.indexOf(pWorker);
    if(index > -1) {
        m_outputWorkers[index].clear();
        kLogger.debug() << "removeWorker: worker removed";
    } else {
        kLogger.warning() << "removeWorker: ERROR: worker not found";
    }
    debugOutputSlots();
}

void EngineNetworkStream::setInputWorker(NetworkInputStreamWorker* pInputWorker) {
    if (pInputWorker) {
        pInputWorker->setSourceFifo(m_pInputFifo);
    }
}

int EngineNetworkStream::nextOutputSlotAvailable() {
    return m_outputWorkers.indexOf(NetworkOutputStreamWorkerPtr(nullptr));
}

void EngineNetworkStream::debugOutputSlots() {
    int available = m_outputWorkers.count(NetworkOutputStreamWorkerPtr(nullptr));
    int total = m_outputWorkers.size();
    kLogger.debug() << "worker slots used:"
                    << QString("%1 out of %2").arg(total - available).arg(total);
}
