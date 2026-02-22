#pragma once

#include <QMutex>
#include <QString>

#include "audio/frame.h"
#include "audio/types.h"
#include "engine/cachingreader/cachingreaderchunk.h"
#include "engine/engineworker.h"
#include "sources/audiosource.h"
#include "track/track_decl.h"

template<class DataType>
class FIFO;

// POD with trivial ctor/dtor/copy for passing through FIFO
typedef struct CachingReaderChunkReadRequest {
    CachingReaderChunk* chunk;

    void giveToWorker(CachingReaderChunkForOwner* chunkForOwner) {
        DEBUG_ASSERT(chunkForOwner);
        chunk = chunkForOwner;
        chunkForOwner->giveToWorker();
    }
} CachingReaderChunkReadRequest;

enum ReaderStatus {
    TRACK_LOADED,
    TRACK_UNLOADED,
    CHUNK_READ_SUCCESS,
    CHUNK_READ_EOF,
    CHUNK_READ_INVALID,
    CHUNK_READ_DISCARDED, // response without frame index range!
};

// POD with trivial ctor/dtor/copy for passing through FIFO
typedef struct ReaderStatusUpdate {
  private:
    CachingReaderChunk* chunk;
    SINT readableFrameIndexRangeStart;
    SINT readableFrameIndexRangeEnd;

  public:
    ReaderStatus status;

    void init(
            ReaderStatus statusArg,
            CachingReaderChunk* chunkArg,
            const mixxx::IndexRange& readableFrameIndexRangeArg) {
        status = statusArg;
        chunk = chunkArg;
        readableFrameIndexRangeStart = readableFrameIndexRangeArg.start();
        readableFrameIndexRangeEnd = readableFrameIndexRangeArg.end();
    }

    static ReaderStatusUpdate readDiscarded(
            CachingReaderChunk* chunk) {
        ReaderStatusUpdate update;
        update.init(CHUNK_READ_DISCARDED, chunk, mixxx::IndexRange());
        return update;
    }

    static ReaderStatusUpdate trackLoaded(
            const mixxx::IndexRange& readableFrameIndexRange) {
        DEBUG_ASSERT(!readableFrameIndexRange.empty());
        ReaderStatusUpdate update;
        update.init(TRACK_LOADED, nullptr, readableFrameIndexRange);
        return update;
    }

    static ReaderStatusUpdate trackUnloaded() {
        ReaderStatusUpdate update;
        update.init(TRACK_UNLOADED, nullptr, mixxx::IndexRange());
        return update;
    }

    CachingReaderChunkForOwner* takeFromWorker() {
        CachingReaderChunkForOwner* pChunk = nullptr;
        if (chunk) {
            DEBUG_ASSERT(dynamic_cast<CachingReaderChunkForOwner*>(chunk));
            pChunk = static_cast<CachingReaderChunkForOwner*>(chunk);
            chunk = nullptr;
            pChunk->takeFromWorker();
        }
        return pChunk;
    }

    mixxx::IndexRange readableFrameIndexRange() const {
        return mixxx::IndexRange::between(
                readableFrameIndexRangeStart,
                readableFrameIndexRangeEnd);
    }
} ReaderStatusUpdate;

class CachingReaderWorker : public EngineWorker {
    Q_OBJECT

  public:
    // Construct a CachingReader with the given group.
    CachingReaderWorker(const QString& group,
            FIFO<CachingReaderChunkReadRequest>* pChunkReadRequestFIFO,
            FIFO<ReaderStatusUpdate>* pReaderStatusFIFO);
    ~CachingReaderWorker() override = default;

    // Request to load a new track. wake() must be called afterwards.
    void newTrack(TrackPointer pTrack);

    // Run upkeep operations like loading tracks and reading from file. Run by a
    // thread pool via the EngineWorkerScheduler.
    void run() override;

    void quitWait();

  signals:
    // Emitted once a new track is loaded and ready to be read from.
    void trackLoading();
    void trackLoaded(TrackPointer pTrack, mixxx::audio::SampleRate sampleRate, double numSamples);
    void trackLoadFailed(TrackPointer pTrack, const QString& reason);

  private:
    const QString m_group;
    QString m_tag;

    // Thread-safe FIFOs for communication between the engine callback and
    // reader thread.
    FIFO<CachingReaderChunkReadRequest>* m_pChunkReadRequestFIFO;
    FIFO<ReaderStatusUpdate>* m_pReaderStatusFIFO;

    // Queue of Tracks to load, and the corresponding lock. Must acquire the
    // lock to touch.
    QMutex m_newTrackMutex;
    QAtomicInt m_newTrackAvailable;
    TrackPointer m_pNewTrack;

    void discardAllPendingRequests();

    /// call to be prepare for new tracks
    /// Make sure engine has been stopped before
    void closeAudioSource();

    /// Internal method to unload a track.
    /// does not emit signals
    void unloadTrack();

    /// Internal method to load a track. Emits trackLoaded when finished.
    void loadTrack(const TrackPointer& pTrack);

    ReaderStatusUpdate processReadRequest(
            const CachingReaderChunkReadRequest& request);

    void verifyFirstSound(const CachingReaderChunk* pChunk);

    // The current audio source of the track loaded
    mixxx::AudioSourcePointer m_pAudioSource;

    mixxx::audio::FramePos m_firstSoundFrameToVerify;

    // Temporary buffer for reading samples from all channels
    // before conversion to a stereo signal.
    mixxx::SampleBuffer m_tempReadBuffer;

    QAtomicInt m_stop;
};
