#include "engine/cachingreader/cachingreaderworker.h"

#include <QAtomicInt>
#include <QtDebug>

#include "analyzer/analyzersilence.h"
#include "moc_cachingreaderworker.cpp"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/compatibility/qmutex.h"
#include "util/event.h"
#include "util/fifo.h"
#include "util/logger.h"
#include "util/span.h"

namespace {

mixxx::Logger kLogger("CachingReaderWorker");

// we need the last silence frame and the first sound frame
constexpr SINT kNumSoundFrameToVerify = 2;

} // anonymous namespace

CachingReaderWorker::CachingReaderWorker(
        const QString& group,
        FIFO<CachingReaderChunkReadRequest>* pChunkReadRequestFIFO,
        FIFO<ReaderStatusUpdate>* pReaderStatusFIFO,
        mixxx::audio::ChannelCount maxSupportedChannel)
        : m_group(group),
          m_tag(QString("CachingReaderWorker %1").arg(m_group)),
          m_pChunkReadRequestFIFO(pChunkReadRequestFIFO),
          m_pReaderStatusFIFO(pReaderStatusFIFO),
          m_maxSupportedChannel(maxSupportedChannel) {
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
            chunkFrameIndexRange.isSubrangeOf(m_pAudioSource->frameIndexRange()));
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
            bufferedFrameIndexRange.isSubrangeOf(m_pAudioSource->frameIndexRange()));
    // The readable frame range might have changed
    chunkFrameIndexRange = intersect(chunkFrameIndexRange, m_pAudioSource->frameIndexRange());
    DEBUG_ASSERT(bufferedFrameIndexRange.empty() ||
            bufferedFrameIndexRange.isSubrangeOf(chunkFrameIndexRange));

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

    // This call here assumes that the caching reader will read the first sound cue at
    // one of the first chunks. The check serves as a sanity check to ensure that the
    // sample data has not changed since it has ben analyzed. This could happen because
    // of a change in actual audio data or because the file was decoded using a different
    // decoder
    // This is part of a first prove of concept and needs to be replaces with a different
    // solution which is still under discussion. This might be also extended
    // to further checks whether a automatic offset adjustment is possible or a the
    // sample position metadata shall be treated as outdated.
    // Failures of the sanity check only result in an entry into the log at the moment.
    verifyFirstSound(pChunk, m_pAudioSource->getSignalInfo().getChannelCount());

    ReaderStatusUpdate result;
    result.init(status, pChunk, m_pAudioSource ? m_pAudioSource->frameIndexRange() : mixxx::IndexRange());
    return result;
}

// WARNING: Always called from a different thread (GUI)
#ifdef __STEM__
void CachingReaderWorker::newTrack(TrackPointer pTrack, mixxx::StemChannelSelection stemMask) {
#else
void CachingReaderWorker::newTrack(TrackPointer pTrack) {
#endif
    {
        const auto locker = lockMutex(&m_newTrackMutex);
#ifdef __STEM__
        m_pNewTrack = NewTrackRequest{
                pTrack,
                stemMask};
#else
        m_pNewTrack = pTrack;
#endif
        m_newTrackAvailable.storeRelease(1);
    }
    workReady();
}

void CachingReaderWorker::run() {
    // the id of this thread, for debugging purposes
    static auto lastId = QAtomicInt(0);
    const auto id = lastId.fetchAndAddRelaxed(1) + 1;
    QThread::currentThread()->setObjectName(
            QStringLiteral("CachingReaderWorker ") + QString::number(id));

    Event::start(m_tag);
    while (!m_stop.loadAcquire()) {
        // Request is initialized by reading from FIFO
        CachingReaderChunkReadRequest request;
        if (m_newTrackAvailable.loadAcquire()) {
#ifdef __STEM__
            NewTrackRequest pLoadTrack;
#else
            TrackPointer pLoadTrack;
#endif
            { // locking scope
                const auto locker = lockMutex(&m_newTrackMutex);
                pLoadTrack = m_pNewTrack;
                m_newTrackAvailable.storeRelease(0);
            } // implicitly unlocks the mutex
#ifdef __STEM__
            if (pLoadTrack.track) {
                // in this case the engine is still running with the old track
                loadTrack(pLoadTrack.track, pLoadTrack.stemMask);
#else
            if (pLoadTrack) {
                // in this case the engine is still running with the old track
                loadTrack(pLoadTrack);
#endif
            } else {
                // here, the engine is already stopped
                unloadTrack();
            }
        } else if (m_pChunkReadRequestFIFO->read(&request, 1) == 1) {
            // Read the requested chunk and send the result
            const ReaderStatusUpdate update = processReadRequest(request);
            m_pReaderStatusFIFO->writeBlocking(&update, 1);
        } else {
            Event::end(m_tag);
            m_semaRun.acquire();
            Event::start(m_tag);
        }
    }
}

void CachingReaderWorker::discardAllPendingRequests() {
    CachingReaderChunkReadRequest request;
    while (m_pChunkReadRequestFIFO->read(&request, 1) == 1) {
        const auto update = ReaderStatusUpdate::readDiscarded(request.chunk);
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
    }
}

void CachingReaderWorker::closeAudioSource() {
    discardAllPendingRequests();

    if (m_pAudioSource) {
        // Closes open file handles of the old track.
        m_pAudioSource->close();
        m_pAudioSource.reset();
    }

    // This function has to be called with the engine stopped only
    // to avoid collecting new requests for the old track
    DEBUG_ASSERT(!m_pChunkReadRequestFIFO->readAvailable());
}

void CachingReaderWorker::unloadTrack() {
    closeAudioSource();

    const auto update = ReaderStatusUpdate::trackUnloaded();
    m_pReaderStatusFIFO->writeBlocking(&update, 1);
}

#ifdef __STEM__
void CachingReaderWorker::loadTrack(
        const TrackPointer& pTrack, mixxx::StemChannelSelection stemMask) {
#else
void CachingReaderWorker::loadTrack(const TrackPointer& pTrack) {
#endif
    // This emit is directly connected and returns synchronized
    // after the engine has been stopped.
    emit trackLoading();

    closeAudioSource();

    if (!pTrack->getFileInfo().checkFileExists()) {
        kLogger.warning()
                << m_group
                << "File not found"
                << pTrack->getFileInfo();
        const auto update = ReaderStatusUpdate::trackUnloaded();
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
        emit trackLoadFailed(pTrack,
                tr("The file '%1' could not be found.")
                        .arg(QDir::toNativeSeparators(pTrack->getLocation())));
        return;
    }

    mixxx::AudioSource::OpenParams config;
    config.setChannelCount(m_maxSupportedChannel);
#ifdef __STEM__
    config.setStemMask(stemMask);
#endif
    m_pAudioSource = SoundSourceProxy(pTrack).openAudioSource(config);
    if (!m_pAudioSource) {
        kLogger.warning()
                << m_group
                << "Failed to open file"
                << pTrack->getFileInfo();
        const auto update = ReaderStatusUpdate::trackUnloaded();
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
        emit trackLoadFailed(pTrack,
                tr("The file '%1' could not be loaded.")
                        .arg(QDir::toNativeSeparators(pTrack->getLocation())));
        return;
    }

    // It is critical that the audio source doesn't contain more channels than
    // requested as this could lead to overflow when reading chunks
    VERIFY_OR_DEBUG_ASSERT(m_pAudioSource->getSignalInfo().getChannelCount() >=
                    mixxx::audio::ChannelCount::mono() &&
            m_pAudioSource->getSignalInfo().getChannelCount() <=
                    m_maxSupportedChannel) {
        m_pAudioSource.reset(); // Close open file handles
        const auto update = ReaderStatusUpdate::trackUnloaded();
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
        emit trackLoadFailed(pTrack,
                tr("The file '%1' could not be loaded because it contains %2 "
                   "channels, and only 1 to %3 are supported.")
                        .arg(QDir::toNativeSeparators(pTrack->getLocation()),
                                QString::number(m_pAudioSource->getSignalInfo()
                                                .getChannelCount()),
                                QString::number(m_maxSupportedChannel)));
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
                << pTrack->getFileInfo();
        const auto update = ReaderStatusUpdate::trackUnloaded();
        m_pReaderStatusFIFO->writeBlocking(&update, 1);
        emit trackLoadFailed(pTrack,
                tr("The file '%1' is empty and could not be loaded.")
                        .arg(QDir::toNativeSeparators(pTrack->getLocation())));
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

    // This code is a workaround until we have found a better solution to
    // verify and correct offsets.
    CuePointer pN60dBSound =
            pTrack->findCueByType(mixxx::CueType::N60dBSound);
    if (pN60dBSound) {
        m_firstSoundFrameToVerify = pN60dBSound->getPosition();
    }

    // The engine must not request any chunks before receiving the
    // trackLoaded() signal
    DEBUG_ASSERT(!m_pChunkReadRequestFIFO->readAvailable());

    emit trackLoaded(
            pTrack,
            m_pAudioSource->getSignalInfo().getSampleRate(),
            m_pAudioSource->getSignalInfo().getChannelCount(),
            mixxx::audio::FramePos(m_pAudioSource->frameLength()));
}

void CachingReaderWorker::quitWait() {
    m_stop = 1;
    m_semaRun.release();
    wait();
}

void CachingReaderWorker::verifyFirstSound(const CachingReaderChunk* pChunk,
        mixxx::audio::ChannelCount channelCount) {
    if (!m_firstSoundFrameToVerify.isValid()) {
        return;
    }

    const int firstSoundIndex =
            CachingReaderChunk::indexForFrame(static_cast<SINT>(
                    m_firstSoundFrameToVerify.toLowerFrameBoundary()
                            .value()));
    if (pChunk->getIndex() == firstSoundIndex) {
        mixxx::SampleBuffer sampleBuffer(kNumSoundFrameToVerify * channelCount);
        SINT end = static_cast<SINT>(m_firstSoundFrameToVerify.toLowerFrameBoundary().value());
        pChunk->readBufferedSampleFrames(sampleBuffer.data(),
                channelCount,
                mixxx::IndexRange::forward(end - 1, kNumSoundFrameToVerify));
        if (AnalyzerSilence::verifyFirstSound(sampleBuffer.span(),
                    mixxx::audio::FramePos(1),
                    channelCount)) {
            qDebug() << "First sound found at the previously stored position";
        } else {
            // This can happen in case of track edits or replacements, changed
            // encoders or encoding issues.
            qWarning() << "First sound has been moved! The beatgrid and "
                          "other annotations are no longer valid"
                       << m_pAudioSource->getUrlString();
        }
        m_firstSoundFrameToVerify = mixxx::audio::FramePos();
    }
}
