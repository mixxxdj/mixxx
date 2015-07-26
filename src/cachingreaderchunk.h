#ifndef CACHINGREADERCHUNK_H
#define CACHINGREADERCHUNK_H

#include "sources/audiosource.h"


// A Chunk is a memory-resident section of audio that has been cached.
// Each chunk holds a fixed number kFrames of frames with samples for
// kChannels.
// The class is not thread-safe although it is shared between CachingReader
// and CachingReaderWorker! A lock-free FIFO ensures that only a single
// thread has exclusive access on each chunk. The state READ_PENDING
// indicates that the worker thread is in control.
class CachingReaderChunk {
public:
    static const SINT kDefaultIndex;
    static const SINT kChannels;
    static const SINT kFrames;
    static const SINT kSamples;

    // Returns the corresponding chunk index for a frame index
    inline static SINT indexForFrame(SINT frameIndex) {
        DEBUG_ASSERT(Mixxx::AudioSource::getMinFrameIndex() <= frameIndex);
        const SINT chunkIndex = frameIndex / kFrames;
        return chunkIndex;
    }

    // Returns the corresponding chunk index for a frame index
    inline static SINT frameForIndex(SINT chunkIndex) {
        DEBUG_ASSERT(0 <= chunkIndex);
        return chunkIndex * kFrames;
    }

    // Converts frames to samples
    inline static SINT frames2samples(SINT frames) {
        return frames * kChannels;
    }
    // Converts samples to frames
    inline static SINT samples2frames(SINT samples) {
        DEBUG_ASSERT(0 == (samples % kChannels));
        return samples / kChannels;
    }

    explicit CachingReaderChunk(CSAMPLE* sampleBuffer);
    virtual ~CachingReaderChunk();

    void init(SINT index);
    void free();

    enum State {
        FREE,
        READY,
        READ_PENDING
    };

    State getState() const {
        return m_state;
    }

    // Please note that the state must exclusively be controlled by the
    // cache and not the worker thread!
    void beginReading() {
        DEBUG_ASSERT(READY == m_state);
        m_state = READ_PENDING;
    }
    void finishReading() {
        DEBUG_ASSERT(READ_PENDING == m_state);
        m_state = READY;
    }

    // Inserts a chunk into the double-linked list before the
    // given element
    void insertIntoListBefore(
            CachingReaderChunk* pBefore);
    // Removes a chunk from the double-linked list and optionally
    // adjusts head/tail pointers
    void removeFromList(
            CachingReaderChunk** ppHead,
            CachingReaderChunk** ppTail);

    SINT getIndex() const {
        return m_index;
    }

    bool isValid() const {
        return 0 <= getIndex();
    }

    SINT getFrameCount() const {
        return m_frameCount;
    }

    // Check if the audio source has sample data available
    // for this chunk.
    bool isReadable(
            const Mixxx::AudioSourcePointer& pAudioSource,
            SINT maxReadableFrameIndex) const;

    // Read sample frames from the audio source and return the
    // number of frames that have been read. The in/out parameter
    // pMaxReadableFrameIndex is adjusted if reading fails.
    SINT readSampleFrames(
            const Mixxx::AudioSourcePointer& pAudioSource,
            SINT* pMaxReadableFrameIndex);

    // Copy sampleCount samples starting at sampleOffset from
    // the chunk's internal buffer into sampleBuffer.
    void copySamples(
            CSAMPLE* sampleBuffer,
            SINT sampleOffset,
            SINT sampleCount) const;

private:
    SINT m_index;

    CachingReaderChunk* m_pPrev; // in double-linked list
    CachingReaderChunk* m_pNext; // in double-linked list

    // The state is read from both the cache and the worker thread.
    // But only the cache is allowed to modify the state.
    volatile State m_state;

    // The worker thread will fill the sample buffer and
    // set the frame count.
    CSAMPLE* const m_sampleBuffer;
    SINT m_frameCount;
};


#endif // CACHINGREADERCHUNK_H
