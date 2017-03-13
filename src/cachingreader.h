// cachingreader.h
// Created 7/9/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CACHINGREADER_H
#define CACHINGREADER_H

#include <QtDebug>
#include <QList>
#include <QVector>
#include <QLinkedList>
#include <QHash>
#include <QVarLengthArray>

#include "util/types.h"
#include "configobject.h"
#include "trackinfoobject.h"
#include "engine/engineworker.h"
#include "util/fifo.h"
#include "cachingreaderworker.h"

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

// Note that we use a QVarLengthArray here instead of a QVector. Since this list
// is cleared on every callback and potentially referenced multiples times it's
// nicer to use a QVarLengthArray over a QVector because of two things:
//
// 1) No copy-on-write / implicit sharing behavior. If the reference count rises
//    above 1 then every non-const operation on a QVector clones it. We'd like
//    to avoid unnecessary memory allocation in the callback thread so this is
//    undesirable.
// 2) QVector::clear deletes the backing store (even if you call reserve) so we
//    reallocate on every callback. resize(0) should work but a future developer
//    may see a resize(0) and say "that's a silly way of writing clear()!" and
//    replace it without realizing.
typedef QVarLengthArray<Hint, 512> HintVector;

// CachingReader provides a layer on top of a SoundSource for reading samples
// from a file. Since we cannot do file I/O in the audio callback thread
// CachingReader and CachingReaderWorker (a worker thread) work in concert to
// read and decode relevant sections of a track in a background thread. The
// decoded chunks are kept in a cache by CachingReader with a
// least-recently-used (LRU) eviction policy. CachingReader exposes a method for
// indicating which chunks should be kept fresh in the cache (see
// hintAndMaybeWake). For example, the chunks around the playhead, the hotcue
// positions, and loop points are all portions of the track that the user is
// likely to dynamically jump to so we should keep them ready.
//
// The least recently used policy is implemented by keeping a linked list of the
// least recently used chunks. When a chunk is "freshened" (i.e. accessed via
// read or hinted via hintAndMaybeWake) then it is moved to the back of the
// least-recently-used list. When a chunk needs to be allocated and there are no
// free chunks then the least recently used chunk is free'd (see
// allocateChunkExpireLRU).
class CachingReader : public QObject {
    Q_OBJECT

  public:
    // Construct a CachingReader with the given group.
    CachingReader(QString group,
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
    virtual void hintAndMaybeWake(const HintVector& hintList);

    // Request that the CachingReader load a new track. These requests are
    // processed in the work thread, so the reader must be woken up via wake()
    // for this to take effect.
    virtual void newTrack(TrackPointer pTrack);

    void setScheduler(EngineWorkerScheduler* pScheduler) {
        m_pWorker->setScheduler(pScheduler);
    }

    const static int maximumChunksInMemory;

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
        return sample_number / CachingReaderWorker::kSamplesPerChunk;
    }

    const ConfigObject<ConfigValue>* m_pConfig;

    // Thread-safe FIFOs for communication between the engine callback and
    // reader thread.
    FIFO<ChunkReadRequest> m_chunkReadRequestFIFO;
    FIFO<ReaderStatusUpdate> m_readerStatusFIFO;

    // Looks for the provided chunk number in the index of in-memory chunks and
    // returns it if it is present. If not, returns NULL. If it is present then
    // freshenChunk is called on the chunk to make it the MRU chunk.
    Chunk* lookupChunkAndFreshen(int chunk_number);

    // Looks for the provided chunk number in the index of in-memory chunks and
    // returns it if it is present. If not, returns NULL.
    Chunk* lookupChunk(int chunk_number);

    // Moves the provided chunk to the MRU position.
    void freshenChunk(Chunk* pChunk);

    // Returns a Chunk to the free list
    void freeChunk(Chunk* pChunk);

    // Returns all allocated chunks to the free list
    void freeAllChunks();

    // Gets a chunk from the free list. Returns NULL if none available.
    Chunk* allocateChunk(int chunk);

    // Gets a chunk from the free list, frees the LRU Chunk if none available.
    Chunk* allocateChunkExpireLRU(int chunk);

    ReaderStatus m_readerStatus;

    // Keeps track of all Chunks we've allocated.
    QVector<Chunk*> m_chunks;

    // List of free chunks. Linked list so that we have constant time insertions
    // and deletions. Iteration is not necessary.
    QLinkedList<Chunk*> m_freeChunks;

    // Keeps track of what Chunks we've allocated and indexes them based on what
    // chunk number they are allocated to.
    QHash<int, Chunk*> m_allocatedChunks;

    // The linked list of recently-used chunks.
    Chunk* m_mruChunk;
    Chunk* m_lruChunk;

    // The raw memory buffer which is divided up into chunks.
    CSAMPLE* m_pRawMemoryBuffer;

    int m_iTrackNumSamplesCallbackSafe;

    CachingReaderWorker* m_pWorker;
};


#endif /* CACHINGREADER_H */
