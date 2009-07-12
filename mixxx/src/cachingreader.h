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

class SoundSource;
class TrackInfoObject;


typedef struct Hint {
    int sample;
    int length;
    int priority;
} Hint;

typedef struct Chunk {
    int chunk_number;
    int sample;
    int length;
    CSAMPLE* data;
} Chunk;

class CachingReader : public QThread {
    Q_OBJECT

    public:

    CachingReader(const char* _group,
                  ConfigObject<ConfigValue>* _config);
    virtual ~CachingReader();

    // You really shouldn't use these unless there /really/ isn't any other way
    // of getting at this data.
    int getTrackSampleRate();
    int getTrackNumSamples();

    int read(int sample, int num_samples, CSAMPLE* buffer);
    void hint(int sample, int length, int priority);
    void newTrack(TrackInfoObject* pTrack);
    void wake();

signals:
    void trackLoaded(TrackInfoObject *pTrack, int iSampleRate, int iNumSamples);
    
protected:
    void run();

private:

    const static int kChunkLength, kSamplesPerChunk;

    void initialize();
    void stop();

    void loadTrack(TrackInfoObject *pTrack);
    
    // Queue of recent hints, and the corresponding lock.
    QMutex m_hintQueueMutex;
    QQueue<Hint> m_hintQueue;

    // Queue of Tracks to load, and the corresponding lock.
    QMutex m_trackQueueMutex;
    QQueue<TrackInfoObject*> m_trackQueue;
    
    // Held when the Reader is working: read() vs. run()
    QMutex m_readerMutex;
    QWaitCondition m_readerWait;

    // 
    // Everything below this line is guarded by m_readerMutex, make sure you
    // hold it if you touch them.
    //

    // Look up chunk_number at any cost. Reads from SoundSource if
    // necessary. Returns NULL if chunk_number is not valid.
    Chunk* getChunk(int chunk_number);
    // Read the given chunk_number from the file into pChunk's data
    // buffer. Fills length/sample information about Chunk* as well.
    bool readChunkFromFile(Chunk* pChunk, int chunk_number);
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

    int chunkForSample(int sample_number) {
        // TODO make sure this floor()'s it
        return sample_number / kSamplesPerChunk;
    }
    
    int sampleForChunk(int chunk_number) {
        return chunk_number * kSamplesPerChunk;
    }

    const char* m_pGroup;
    const ConfigObject<ConfigValue>* m_pConfig;

    TrackInfoObject* m_pCurrentTrack;
    SoundSource* m_pCurrentSoundSource;
    int m_iTrackSampleRate;
    int m_iTrackNumSamples;

    // Keeps track of free Chunks we've allocated
    QVector<Chunk> m_chunks;
    QList<Chunk*> m_freeChunks;
    // Keeps track of what Chunks we've allocated and indexes them based on what
    // chunk number they are allocated to.
    QHash<int, Chunk*> m_allocatedChunks;
    QLinkedList<Chunk*> m_recentlyUsedChunks;

    char* m_pRawMemoryBuffer;
    int m_iRawMemoryBufferLength;
    SAMPLE* m_pSample;
    bool m_bQuit;
};


#endif /* CACHINGREADER_H */
