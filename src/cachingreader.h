// cachingreader.h
// Created 7/9/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CACHINGREADER_H
#define CACHINGREADER_H

#include <QtDebug>
#include <QList>
#include <QVector>
#include <QLinkedList>
#include <QHash>

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

// CachingReader provides a layer on top of a SoundSource for reading samples
// from a file. A cache is provided so that repeated reads to a certain section
// of a song do not cause disk seeks or unnecessary SoundSource
// calls. CachingReader provides a worker thread that can be used to prepare the
// cache so that areas of a file that will soon be read are present in memory
// once they are needed. This can be accomplished by issueing 'hints' to the
// reader of areas of a SoundSource that will be read soon.
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
    virtual void hintAndMaybeWake(const QVector<Hint>& hintList);

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

    int m_iTrackNumSamplesCallbackSafe;

    CachingReaderWorker* m_pWorker;
};


#endif /* CACHINGREADER_H */
