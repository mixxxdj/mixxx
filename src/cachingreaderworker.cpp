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

// There's a little math to this, but not much: 48khz stereo audio is 384kb/sec
// if using float samples. We want the chunk size to be a power of 2 so it's
// easier to memory align, and roughly 1/2 - 1/4th of a second of audio. 2**17
// and 2**16 are nice candidates. 2**16 is 170ms of audio, which is well above
// (hopefully) the latencies people are seeing. at 10ms latency, one chunk is
// enough for 17 callbacks. We may need to tweak this later.

// Must be divisible by 8, 4, and 2. Just pick a power of 2.
#define CHUNK_LENGTH 65536

const Mixxx::AudioSource::size_type CachingReaderWorker::kChunkLength = CHUNK_LENGTH;
const Mixxx::AudioSource::size_type CachingReaderWorker::kSamplesPerChunk = CHUNK_LENGTH / sizeof(Mixxx::AudioSource::sample_type);
const Mixxx::AudioSource::size_type CachingReaderWorker::kFramesPerChunk = kSamplesPerChunk / kChunkChannels;


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

void CachingReaderWorker::processChunkReadRequest(ChunkReadRequest* request,
        ReaderStatusUpdate* update) {
    int chunk_number = request->chunk->chunk_number;
    //qDebug() << "Processing ChunkReadRequest for" << chunk_number;
    update->chunk = request->chunk;
    update->chunk->frameCount = 0;

    if (!m_pAudioSource || chunk_number < 0) {
        update->status = CHUNK_READ_INVALID;
        return;
    }

    // Stereo samples
    Mixxx::AudioSource::size_type frame_position = frameForChunk(chunk_number);
    Mixxx::AudioSource::size_type frames_remaining = m_pAudioSource->getFrameCount() - frame_position;
    Mixxx::AudioSource::size_type frames_to_read = math_min(kFramesPerChunk, frames_remaining);

    // Bogus chunk number
    if (frames_to_read <= 0) {
        update->status = CHUNK_READ_EOF;
        return;
    }

    frame_position = m_pAudioSource->seekFrame(frame_position);

    const Mixxx::AudioSource::size_type frames_read = m_pAudioSource->readStereoFrameSamplesInterleaved(frames_to_read, request->chunk->stereoSamples);

    // If we've run out of music, the AudioSource can return 0 frames/samples.
    // Remember that AudioSource->getFrameCount() can lie to us about
    // the length of the song!
    if (frames_read <= 0) {
        update->status = CHUNK_READ_EOF;
        return;
    }

    update->status = CHUNK_READ_SUCCESS;
    update->chunk->frameCount = frames_read;
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
        Mixxx::AudioSourcePointer pAudioSource(soundSourceProxy.open());
        if (pAudioSource.isNull()) {
            qWarning() << "Failed to open file:" << pTrack->getLocation();
            return Mixxx::AudioSourcePointer();
        }
        if (pAudioSource->isFrameCountEmpty()) {
            qWarning() << "File is empty:" << pTrack->getLocation();
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
    emit(trackLoaded(pTrack, m_pAudioSource->getSampleRate(), m_pAudioSource->getFrameCount() * kChunkChannels));
}

void CachingReaderWorker::quitWait() {
    m_stop = 1;
    m_semaRun.release();
    wait();
}
