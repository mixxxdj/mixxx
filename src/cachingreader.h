// cachingreader.h
// Created 7/9/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CACHINGREADER_H
#define CACHINGREADER_H

#include <QDebug>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <QList>
#include <QVector>
#include <QLinkedList>
#include <QHash>
#include <QThread>

#include "defs.h"
#include "configobject.h"
#include "trackinfoobject.h"
#include "engine/engineworker.h"
#include "util/fifo.h"

class ControlObjectThread;
namespace Mixxx {
    class SoundSource;
}

// A Hint is an indication to the CachingReader that a certain section of a
// SoundSource will be used 'soon' and so it should be brought into memory by
// the reader work thread.
typedef struct Hint {
    // The sample to ensure is present in memory.
    int sample;
    // If a range of samples should be present, use length to indicate that the
    // range (sample, sample+length) should be present in memory.
    int length;
    // Currently unused -- but in the future could be used to prioritize certain
    // hints over others. A priority of 1 is the highest priority and should be
    // used for samples that will be read imminently. Hints for samples that
    // have the potential to be read (i.e. a cue point) should be issued with
    // priority >10.
    int priority;
} Hint;

// A Chunk is a section of audio that is being cached. The chunk_number can be
// used to figure out the sample number of the first sample in data by using
// sampleForChunk()
typedef struct Chunk {
    int chunk_number;
    int length;
    CSAMPLE* data;
    Chunk* prev_lru;
    Chunk* next_lru;
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
    CHUNK_READ_EOF,
    CHUNK_READ_INVALID
};

typedef struct ReaderStatusUpdate {
    ReaderStatus status;
    Chunk* chunk;
    int trackNumSamples;
    ReaderStatusUpdate() {
        status = INVALID;
        chunk = NULL;
    }
} ReaderStatusUpdate;

// CachingReader provides a layer on top of a SoundSource for reading samples
// from a file. A cache is provided so that repeated reads to a certain section
// of a song do not cause disk seeks or unnecessary SoundSource
// calls. CachingReader provides a worker thread that can be used to prepare the
// cache so that areas of a file that will soon be read are present in memory
// once they are needed. This can be accomplished by issueing 'hints' to the
// reader of areas of a SoundSource that will be read soon.
class CachingReader : public EngineWorker {
    Q_OBJECT

  public:
    // Construct a CachingReader with the given group.
    CachingReader(const char* _group,
                  ConfigObject<ConfigValue>* _config);
    virtual ~CachingReader();

    virtual void process();

    // Read num_samples from the SoundSource starting with sample into
    // buffer. Returns the total number of samples actually written to buffer.
    virtual int read(int sample, int num_samples, CSAMPLE* buffer);

    // Issue a list of hints, but check whether any of the hints request a chunk
    // that is not in the cache. If any hints do request a chunk not in cache,
    // then wake the reader so that it can process them. Must only be called
    // from the engine callback.
    virtual void hintAndMaybeWake(QList<Hint>& hintList);

    // Request that the CachingReader load a new track. These requests are
    // processed in the work thread, so the reader must be woken up via wake()
    // for this to take effect.
    virtual void newTrack(TrackPointer pTrack);

    // Wake the reader up so that it will process newTrack requests and hints.
    virtual void wake();

    // Run upkeep operations like loading tracks and reading from file. Run by a
    // thread pool via the EngineWorkerScheduler.
    virtual void run();

    // A Chunk is a memory-resident section of audio that has been cached. Each
    // chunk holds a fixed number of samples given by kSamplesPerChunk.
    const static int kChunkLength, kSamplesPerChunk;

  signals:
    // Emitted once a new track is loaded and ready to be read from.
    void trackLoading();
    void trackLoaded(TrackPointer pTrack, int iSampleRate, int iNumSamples);
    void trackLoadFailed(TrackPointer pTrack, QString reason);

  private:
    // Removes a chunk from the LRU list
    static Chunk* removeFromLRUList(Chunk* chunk, Chunk* head);
    // Inserts a chunk into the LRU list
    static Chunk* insertIntoLRUList(Chunk* chunk, Chunk* head);

    // Given a sample number, return the chunk number corresponding to it.
    inline static int chunkForSample(int sample_number) {
        return sample_number / kSamplesPerChunk;
    }

    // Given a chunk number, return the start sample number for the chunk.
    inline static int sampleForChunk(int chunk_number) {
        return chunk_number * kSamplesPerChunk;
    }

    // Initialize the reader by creating all the chunks from the RAM provided to
    // the CachingReader.
    void initialize();

    const char* m_pGroup;
    const ConfigObject<ConfigValue>* m_pConfig;

    // Thread-safe FIFOs for communication between the engine callback and
    // reader thread.
    FIFO<ChunkReadRequest> m_chunkReadRequestFIFO;
    FIFO<ReaderStatusUpdate> m_readerStatusFIFO;

    // Queue of Tracks to load, and the corresponding lock. Must acquire the
    // lock to touch.
    QMutex m_newTrackMutex;
    TrackPointer m_newTrack;

    ////////////////////////////////////////////////////////////////////////////
    // The following may /only/ be called within the engine callback
    ////////////////////////////////////////////////////////////////////////////

    // Looks for the provided chunk number in the index of in-memory chunks and
    // returns it if it is present. If not, returns NULL.
    Chunk* lookupChunk(int chunk_number);

    // Returns a Chunk to the free list
    void freeChunk(Chunk* pChunk);

    // Returns all allocated chunks to the free list
    void freeAllChunks();

    // Gets a chunk from the free list. Returns NULL if none available.
    Chunk* allocateChunk();

    // Gets a chunk from the free list, frees the LRU Chunk if none available.
    Chunk* allocateChunkExpireLRU();

    ReaderStatus m_readerStatus;

    // Keeps track of free Chunks we've allocated
    QVector<Chunk*> m_chunks;
    // List of free chunks available for use.
    QList<Chunk*> m_freeChunks;
    // List of reserved chunks with reads in progress
    QHash<int, Chunk*> m_chunksBeingRead;

    // Keeps track of what Chunks we've allocated and indexes them based on what
    // chunk number they are allocated to.
    QHash<int, Chunk*> m_allocatedChunks;

    // The linked list of recently-used chunks.
    Chunk* m_mruChunk;
    Chunk* m_lruChunk;

    // The raw memory buffer which is divided up into chunks.
    CSAMPLE* m_pRawMemoryBuffer;

    ////////////////////////////////////////////////////////////////////////////
    // The following may /only/ be called within the reader thread
    ////////////////////////////////////////////////////////////////////////////

    // Internal method to load a track. Emits trackLoaded when finished.
    void loadTrack(TrackPointer pTrack);

    // Read the given chunk_number from the file into pChunk's data
    // buffer. Fills length/sample information about Chunk* as well.
    void processChunkReadRequest(ChunkReadRequest* request,
                                 ReaderStatusUpdate* update);

    // The current sound source of the track loaded
    Mixxx::SoundSource* m_pCurrentSoundSource;
    int m_iTrackSampleRate;
    int m_iTrackNumSamples;
    int m_iTrackNumSamplesCallbackSafe;

    // Temporary buffer for reading from SoundSources
    SAMPLE* m_pSample;
};


#endif /* CACHINGREADER_H */
