#ifndef CACHINGREADERWORKER_H
#define CACHINGREADERWORKER_H

#include <QtDebug>
#include <QMutex>
#include <QSemaphore>
#include <QThread>
#include <QString>

#include "trackinfoobject.h"
#include "engine/engineworker.h"
#include "sources/audiosource.h"
#include "util/fifo.h"


// forward declaration(s)
class AudioSourceProxy;

// A Chunk is a section of audio that is being cached. The chunk_number can be
// used to figure out the sample number of the first sample in data by using
// sampleForChunk()
typedef struct Chunk {
    int chunk_number;
    int frameCountRead;
    int frameCountTotal;
    CSAMPLE* stereoSamples;
    Chunk* prev_lru;
    Chunk* next_lru;

    enum State {
        FREE,
        ALLOCATED,
        READ_IN_PROGRESS,
        READ
    };
    State state;
} Chunk;

typedef struct ChunkReadRequest {
    Chunk* chunk;

    ChunkReadRequest() { chunk = NULL; }
} ChunkReadRequest;

enum ReaderStatus {
    INVALID,
    TRACK_NOT_LOADED,
    TRACK_LOADED,
    CHUNK_READ_SUCCESS,
    CHUNK_READ_PARTIAL,
    CHUNK_READ_EOF,
    CHUNK_READ_INVALID
};

typedef struct ReaderStatusUpdate {
    ReaderStatus status;
    Chunk* chunk;
    int trackFrameCount;
    ReaderStatusUpdate()
        : status(INVALID)
        , chunk(NULL)
        , trackFrameCount(0) {
    }
} ReaderStatusUpdate;

class CachingReaderWorker : public EngineWorker {
    Q_OBJECT

  public:
    // Construct a CachingReader with the given group.
    CachingReaderWorker(QString group,
            FIFO<ChunkReadRequest>* pChunkReadRequestFIFO,
            FIFO<ReaderStatusUpdate>* pReaderStatusFIFO);
    virtual ~CachingReaderWorker();

    // Request to load a new track. wake() must be called afterwards.
    virtual void newTrack(TrackPointer pTrack);

    // Run upkeep operations like loading tracks and reading from file. Run by a
    // thread pool via the EngineWorkerScheduler.
    virtual void run();

    void quitWait();

    // A Chunk is a memory-resident section of audio that has been cached. Each
    // chunk holds a fixed number of stereo frames given by kFramesPerChunk.
    static const SINT kChunkChannels;
    static const SINT kFramesPerChunk;
    static const SINT kSamplesPerChunk; // = kFramesPerChunk * kChunkChannels

    // Given a chunk number, return the start sample number for the chunk.
    static SINT frameForChunk(SINT chunk_number) {
        return chunk_number * kFramesPerChunk;
    }

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
    FIFO<ChunkReadRequest>* m_pChunkReadRequestFIFO;
    FIFO<ReaderStatusUpdate>* m_pReaderStatusFIFO;

    // Queue of Tracks to load, and the corresponding lock. Must acquire the
    // lock to touch.
    QMutex m_newTrackMutex;
    TrackPointer m_newTrack;

    // Internal method to load a track. Emits trackLoaded when finished.
    void loadTrack(const TrackPointer& pTrack);

    // Read the given chunk_number from the file into pChunk's data
    // buffer. Fills length/sample information about Chunk* as well.
    void processChunkReadRequest(ChunkReadRequest* request,
                                 ReaderStatusUpdate* update);

    // The current audio source of the track loaded
    Mixxx::AudioSourcePointer m_pAudioSource;

    QAtomicInt m_stop;
};


#endif /* CACHINGREADERWORKER_H */
