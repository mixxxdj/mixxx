#include "engine/cachingreader/cachingreaderworker.h"

#include <QFileInfo>
#include <QMutexLocker>
#include <QtDebug>

#include "control/controlobject.h"
#include "sources/soundsourceproxy.h"
#include "util/compatibility.h"
#include "util/event.h"
#include "util/logger.h"

namespace {

mixxx::Logger kLogger("CachingReaderWorker");

} // anonymous namespace

CachingReaderWorker::CachingReaderWorker(
        QString group,
        FIFO<CachingReaderChunkReadRequest>* pChunkReadRequestFIFO,
        FIFO<ReaderStatusUpdate>* pReaderStatusFIFO)
        : m_group(group),
          m_tag(QString("CachingReaderWorker %1").arg(m_group)),
          m_pChunkReadRequestFIFO(pChunkReadRequestFIFO),
          m_pReaderStatusFIFO(pReaderStatusFIFO),
          m_newTrackAvailable(false),
          m_stop(0) {
}

ReaderStatusUpdate CachingReaderWorker::processReadRequest(
        const CachingReaderChunkReadRequest& request) {
    CachingReaderChunk* pChunk = request.chunk;
    DEBUG_ASSERT(pChunk);

    // Before trying to read any data we need to check if the audio source
    // is available and if any audio data that is needed by the chunk is
    // actually available.
    auto chunkFrameIndexRange = pChunk->frameIndexRange(m_pAudioSource);
    DEBUG_ASSERT(!m_pAudioSource ||
            chunkFrameIndexRange <= m_pAudioSource->frameIndexRange());
    if (chunkFrameIndexRange.empty()) {
        ReaderStatusUpdate result;
        result.init(CHUNK_READ_INVALID, pChunk, m_pAudioSource ? m_pAudioSource->frameIndexRange() : mixxx::IndexRange());
        return result;
    }

    // Try to read the data required for the chunk from the audio source
    const mixxx::IndexRange bufferedFrameIndexRange = pChunk->bufferSampleFrames(
            m_pAudioSource,
            mixxx::SampleBuffer::WritableSlice(m_tempReadBuffer));
    DEBUG_ASSERT(!m_pAudioSource ||
            bufferedFrameIndexRange <= m_pAudioSource->frameIndexRange());
    // The readable frame range might have changed
    chunkFrameIndexRange = intersect(chunkFrameIndexRange, m_pAudioSource->frameIndexRange());
    DEBUG_ASSERT(bufferedFrameIndexRange.empty() ||
            bufferedFrameIndexRange <= chunkFrameIndexRange);

    ReaderStatus status = bufferedFrameIndexRange.empty() ? CHUNK_READ_EOF : CHUNK_READ_SUCCESS;
    if (bufferedFrameIndexRange != chunkFrameIndexRange) {
        kLogger.warning()
                << m_group
                << "Failed to read chunk samples for frame index range:"
                << "expected =" << chunkFrameIndexRange
                << ", actual =" << bufferedFrameIndexRange;
        if (bufferedFrameIndexRange.empty()) {
            status = CHUNK_READ_INVALID; // overwrite EOF (see above)
        }
    }

    ReaderStatusUpdate result;
    result.init(status, pChunk, m_pAudioSource ? m_pAudioSource->frameIndexRange() : mixxx::IndexRange());
    return result;
}

// WARNING: Always called from a different thread (GUI)
void CachingReaderWorker::newTrack(TrackPointer pTrack) {
    {
        QMutexLocker locker(&m_newTrackMutex);
        m_pNewTrack = pTrack;
        m_newTrackAvailable = true;
    }
    workReady();
}

void CachingReaderWorker::run() {
    unsigned static id = 0; //the id of this thread, for debugging purposes
    QThread::currentThread()->setObjectName(QString("CachingReaderWorker %1").arg(++id));

    Event::start(m_tag);
    while (!atomicLoadAcquire(m_stop)) {
        // Request is initialized by reading from FIFO
        CachingReaderChunkReadRequest request;
        if (m_newTrackAvailable) {
            TrackPointer pLoadTrack;
            { // locking scope
                QMutexLocker locker(&m_newTrackMutex);
                pLoadTrack = m_pNewTrack;
                m_pNewTrack.reset();
                m_newTrackAvailable = false;
            } // implicitly unlocks the mutex
            loadTrack(pLoadTrack);
        } else if (m_pChunkReadRequestFIFO->read(&request, 1) == 1) {
            // Read the requested chunk and send the result
            const ReaderStatusUpdate update(processReadRequest(request));
            m_pReaderStatusFIFO->writeBlocking(&update, 1);
        } else {
            Event::end(m_tag);
            m_semaRun.acquire();
            Event::start(m_tag);
        }
    }
}

void CachingReaderWorker::loadTrack(const TrackPointer& pTrack) {
    // Discard all pending read requests
    CachingReaderChunkReadRequest request;
    while (m_pChunkReadRequestFIFO->read(&request, 1) == 1) {
        const auto update = ReaderStatusUpdate::readDiscarded(request.chunk);
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
    }

    // Unload the track
    m_pAudioSource.reset(); // Close open file handles

    if (!pTrack) {
        // If no new track is available then we are done
        const auto update = ReaderStatusUpdate::trackUnloaded();
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
        return;
    }

    // Emit that a new track is loading, stops the current track
    emit trackLoading();

    const QString trackLocation = pTrack->getLocation();
    if (trackLocation.isEmpty() || !pTrack->checkFileExists()) {
        kLogger.warning()
                << m_group
                << "File not found"
                << trackLocation;
        const auto update = ReaderStatusUpdate::trackUnloaded();
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
        emit trackLoadFailed(pTrack,
                tr("The file '%1' could not be found.")
                        .arg(QDir::toNativeSeparators(trackLocation)));
        return;
    }

    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(CachingReaderChunk::kChannels);
    m_pAudioSource = SoundSourceProxy(pTrack).openAudioSource(config);
    if (!m_pAudioSource) {
        kLogger.warning()
                << m_group
                << "Failed to open file"
                << trackLocation;
        const auto update = ReaderStatusUpdate::trackUnloaded();
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
        emit trackLoadFailed(pTrack,
                tr("The file '%1' could not be loaded.")
                        .arg(QDir::toNativeSeparators(trackLocation)));
        return;
    }

    // Initially assume that the complete content offered by audio source
    // is available for reading. Later if read errors occur this value will
    // be decreased to avoid repeated reading of corrupt audio data.
    if (m_pAudioSource->frameIndexRange().empty()) {
        m_pAudioSource.reset(); // Close open file handles
        kLogger.warning()
                << m_group
                << "Failed to open empty file"
                << trackLocation;
        const auto update = ReaderStatusUpdate::trackUnloaded();
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
        emit trackLoadFailed(pTrack,
                tr("The file '%1' is empty and could not be loaded.")
                        .arg(QDir::toNativeSeparators(trackLocation)));
        return;
    }

    // Adjust the internal buffer
    const SINT tempReadBufferSize =
            m_pAudioSource->getSignalInfo().frames2samples(
                    CachingReaderChunk::kFrames);
    if (m_tempReadBuffer.size() != tempReadBufferSize) {
        mixxx::SampleBuffer(tempReadBufferSize).swap(m_tempReadBuffer);
    }

    const auto update =
            ReaderStatusUpdate::trackLoaded(
                    m_pAudioSource->frameIndexRange());
    m_pReaderStatusFIFO->writeBlocking(&update, 1);

    // Emit that the track is loaded.
    const SINT sampleCount =
            CachingReaderChunk::frames2samples(
                    m_pAudioSource->frameLength());
    emit trackLoaded(
            pTrack,
            m_pAudioSource->getSignalInfo().getSampleRate(),
            sampleCount);
}

void CachingReaderWorker::quitWait() {
    m_stop = 1;
    m_semaRun.release();
    wait();
}
