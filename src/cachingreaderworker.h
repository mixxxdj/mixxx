#ifndef CACHINGREADERWORKER_H
#define CACHINGREADERWORKER_H

#include <QtDebug>
#include <QMutex>
#include <QSemaphore>
#include <QThread>
#include <QString>

#include "cachingreaderchunk.h"
#include "trackinfoobject.h"
#include "engine/engineworker.h"
#include "sources/audiosource.h"
#include "util/fifo.h"


typedef struct CachingReaderChunkReadRequest {
    CachingReaderChunk* chunk;

    explicit CachingReaderChunkReadRequest(
            CachingReaderChunk* chunkArg = nullptr)
        : chunk(chunkArg) {
    }
} CachingReaderChunkReadRequest;

enum ReaderStatus {
    INVALID,
    TRACK_NOT_LOADED,
    TRACK_LOADED,
    CHUNK_READ_SUCCESS,
    CHUNK_READ_EOF,
    CHUNK_READ_INVALID
};

typedef struct ReaderStatusUpdate {
    ReaderStatus status;
    CachingReaderChunk* chunk;
    SINT maxReadableFrameIndex;
    ReaderStatusUpdate()
        : status(INVALID)
        , chunk(nullptr)
        , maxReadableFrameIndex(Mixxx::AudioSource::getMinFrameIndex()) {
    }
    ReaderStatusUpdate(
            ReaderStatus statusArg,
            CachingReaderChunk* chunkArg,
            SINT maxReadableFrameIndexArg)
        : status(statusArg)
        , chunk(chunkArg)
        , maxReadableFrameIndex(maxReadableFrameIndexArg) {
    }
} ReaderStatusUpdate;

class CachingReaderWorker : public EngineWorker {
    Q_OBJECT

  public:
    // Construct a CachingReader with the given group.
    CachingReaderWorker(QString group,
            FIFO<CachingReaderChunkReadRequest>* pChunkReadRequestFIFO,
            FIFO<ReaderStatusUpdate>* pReaderStatusFIFO);
    virtual ~CachingReaderWorker();

    // Request to load a new track. wake() must be called afterwards.
    virtual void newTrack(TrackPointer pTrack);

    // Run upkeep operations like loading tracks and reading from file. Run by a
    // thread pool via the EngineWorkerScheduler.
    virtual void run();

    void quitWait();

  signals:
    // Emitted once a new track is loaded and ready to be read from.
    void trackLoading();
    void trackLoaded(TrackPointer pTrack, int iSampleRate, int iNumSamples);
    void trackLoadFailed(TrackPointer pTrack, QString reason);

  private:
    QString m_group;
    QString m_tag;

    // Thread-safe FIFOs for communication between the engine callback and
    // reader thread.
    FIFO<CachingReaderChunkReadRequest>* m_pChunkReadRequestFIFO;
    FIFO<ReaderStatusUpdate>* m_pReaderStatusFIFO;

    // Queue of Tracks to load, and the corresponding lock. Must acquire the
    // lock to touch.
    QMutex m_newTrackMutex;
    TrackPointer m_newTrack;

    // Internal method to load a track. Emits trackLoaded when finished.
    void loadTrack(const TrackPointer& pTrack);

    ReaderStatusUpdate processReadRequest(
            const CachingReaderChunkReadRequest& request);

    // The current audio source of the track loaded
    Mixxx::AudioSourcePointer m_pAudioSource;

    // The maximum readable frame index of the AudioSource. Might
    // be adjusted when decoding errors occur to prevent reading
    // the same chunk(s) over and over again.
    // This frame index references the frame that follows the
    // last frame with readable sample data.
    SINT m_maxReadableFrameIndex;

    QAtomicInt m_stop;
};


#endif /* CACHINGREADERWORKER_H */
