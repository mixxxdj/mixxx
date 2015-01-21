#include <QtDebug>
#include <QFileInfo>

#include "controlobject.h"
#include "controlobjectthread.h"

#include "cachingreaderworker.h"
#include "trackinfoobject.h"
#include "soundsourceproxy.h"
#include "util/compatibility.h"
#include "util/event.h"
#include "util/math.h"


// One chunk should contain 1/2 - 1/4th of a second of audio.
// 8192 frames contain about 170 ms of audio at 48 kHz, which
// is well above (hopefully) the latencies people are seeing.
// At 10 ms latency one chunk is enough for 17 callbacks.
// Additionally the chunk size should be a power of 2 for
// easier memory alignment.
// TODO(XXX): The optimum value of the "constant" kFramesPerChunk
// depends on the properties of the AudioSource as the remarks
// above suggest!
const Mixxx::AudioSource::size_type CachingReaderWorker::kChunkChannels = 2; // stereo
const Mixxx::AudioSource::size_type CachingReaderWorker::kFramesPerChunk = 8192; // ~ 170 ms at 48 kHz
const Mixxx::AudioSource::size_type CachingReaderWorker::kSamplesPerChunk = kFramesPerChunk * kChunkChannels;

CachingReaderWorker::CachingReaderWorker(QString group,
        FIFO<ChunkReadRequest>* pChunkReadRequestFIFO,
        FIFO<ReaderStatusUpdate>* pReaderStatusFIFO)
        : m_group(group),
          m_tag(QString("CachingReaderWorker %1").arg(m_group)),
          m_pChunkReadRequestFIFO(pChunkReadRequestFIFO),
          m_pReaderStatusFIFO(pReaderStatusFIFO),
          m_stop(0) {
}

CachingReaderWorker::~CachingReaderWorker() {
}

void CachingReaderWorker::processChunkReadRequest(
        ChunkReadRequest* request,
        ReaderStatusUpdate* update) {
    //qDebug() << "Processing ChunkReadRequest for" << chunk_number;

    // Initialize the output parameter
    update->chunk = request->chunk;
    update->chunk->frameCount = 0;

    const int chunk_number = request->chunk->chunk_number;
    if (!m_pAudioSource || chunk_number < 0) {
        update->status = CHUNK_READ_INVALID;
        return;
    }

    const Mixxx::AudioSource::size_type chunkFrameIndex =
            frameForChunk(chunk_number);
    if (!m_pAudioSource->isValidFrameIndex(chunkFrameIndex)) {
        // Frame index out of range
        update->status = CHUNK_READ_INVALID;
        return;
    }

    const Mixxx::AudioSource::size_type seekFrameIndex =
            m_pAudioSource->seekSampleFrame(chunkFrameIndex);
    DEBUG_ASSERT(m_pAudioSource->isValidFrameIndex(seekFrameIndex));
    const Mixxx::AudioSource::size_type framesRemaining =
            m_pAudioSource->getFrameCount() - seekFrameIndex;
    const Mixxx::AudioSource::size_type framesToRead =
            math_min(kFramesPerChunk, framesRemaining);
    if (0 >= framesToRead) {
        update->status = CHUNK_READ_EOF;
        return;
    }

    const Mixxx::AudioSource::size_type framesRead =
            m_pAudioSource->readSampleFramesStereo(
                    framesToRead, request->chunk->stereoSamples, kSamplesPerChunk);

    // If the AudioSource does not return any samples although
    // there should still be some available at this position
    // a read error must have occurred!
    DEBUG_ASSERT(framesRead <= framesToRead);
    if (0 >= framesRead) {
        update->status = CHUNK_READ_INVALID;
        return;
    }
    // Otherwise all requested samples should have been
    // read entirely (according to the frame calculations
    // above)
    DEBUG_ASSERT(framesRead == framesToRead);

    update->status = CHUNK_READ_SUCCESS;
    update->chunk->frameCount = framesRead;
}

// WARNING: Always called from a different thread (GUI)
void CachingReaderWorker::newTrack(TrackPointer pTrack) {
    m_newTrackMutex.lock();
    m_newTrack = pTrack;
    m_newTrackMutex.unlock();
}

void CachingReaderWorker::run() {
    unsigned static id = 0; //the id of this thread, for debugging purposes
    QThread::currentThread()->setObjectName(QString("CachingReaderWorker %1").arg(++id));

    TrackPointer pLoadTrack;
    ChunkReadRequest request;
    ReaderStatusUpdate status;

    Event::start(m_tag);
    while (!load_atomic(m_stop)) {
        if (m_newTrack) {
            m_newTrackMutex.lock();
            pLoadTrack = m_newTrack;
            m_newTrack = TrackPointer();
            m_newTrackMutex.unlock();
            loadTrack(pLoadTrack);
        } else if (m_pChunkReadRequestFIFO->read(&request, 1) == 1) {
            // Read the requested chunks.
            processChunkReadRequest(&request, &status);
            m_pReaderStatusFIFO->writeBlocking(&status, 1);
        } else {
            Event::end(m_tag);
            m_semaRun.acquire();
            Event::start(m_tag);
        }
    }
}

namespace
{
    Mixxx::AudioSourcePointer openAudioSourceForReading(const TrackPointer& pTrack) {
        SoundSourceProxy soundSourceProxy(pTrack);
        Mixxx::AudioSourcePointer pAudioSource(soundSourceProxy.openAudioSource());
        if (pAudioSource.isNull()) {
            qWarning() << "Failed to open file:" << pTrack->getLocation();
            return Mixxx::AudioSourcePointer();
        }
        // successfully opened and readable
        return pAudioSource;
    }
}

void CachingReaderWorker::loadTrack(const TrackPointer& pTrack) {
    //qDebug() << m_group << "CachingReaderWorker::loadTrack() lock acquired for load.";

    // Emit that a new track is loading, stops the current track
    emit(trackLoading());

    ReaderStatusUpdate status;
    status.status = TRACK_NOT_LOADED;
    status.chunk = NULL;
    status.trackFrameCount = 0;

    QString filename = pTrack->getLocation();

    if (filename.isEmpty() || !pTrack->exists()) {
        // Must unlock before emitting to avoid deadlock
        qDebug() << m_group << "CachingReaderWorker::loadTrack() load failed for\""
                 << filename << "\", unlocked reader lock";
        m_pReaderStatusFIFO->writeBlocking(&status, 1);
        emit(trackLoadFailed(
            pTrack, QString("The file '%1' could not be found.").arg(filename)));
        return;
    }

    m_pAudioSource = openAudioSourceForReading(pTrack);
    if (m_pAudioSource.isNull()) {
        // Must unlock before emitting to avoid deadlock
        qDebug() << m_group << "CachingReaderWorker::loadTrack() load failed for\""
                 << filename << "\", file invalid, unlocked reader lock";
        m_pReaderStatusFIFO->writeBlocking(&status, 1);
        emit(trackLoadFailed(
            pTrack, QString("The file '%1' could not be loaded.").arg(filename)));
        return;
    }

    status.trackFrameCount = m_pAudioSource->getFrameCount();
    status.status = TRACK_LOADED;
    m_pReaderStatusFIFO->writeBlocking(&status, 1);

    // Clear the chunks to read list.
    ChunkReadRequest request;
    while (m_pChunkReadRequestFIFO->read(&request, 1) == 1) {
        qDebug() << "Skipping read request for " << request.chunk->chunk_number;
        status.status = CHUNK_READ_INVALID;
        status.chunk = request.chunk;
        m_pReaderStatusFIFO->writeBlocking(&status, 1);
    }

    // Emit that the track is loaded.
    const Mixxx::AudioSource::size_type sampleCount =
            m_pAudioSource->getFrameCount() * kChunkChannels;
    emit(trackLoaded(pTrack, m_pAudioSource->getFrameRate(), sampleCount));
}

void CachingReaderWorker::quitWait() {
    m_stop = 1;
    m_semaRun.release();
    wait();
}
