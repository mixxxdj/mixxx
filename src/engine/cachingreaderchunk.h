#ifndef ENGINE_CACHINGREADERCHUNK_H
#define ENGINE_CACHINGREADERCHUNK_H

#include "sources/audiosource.h"

// A Chunk is a memory-resident section of audio that has been cached.
// Each chunk holds a fixed number kFrames of frames with samples for
// kChannels.
//
// The class is not thread-safe although it is shared between CachingReader
// and CachingReaderWorker! A lock-free FIFO ensures that only a single
// thread has exclusive access on each chunk. This abstract base class
// is available for both the worker thread and the cache.
//
// This is the common (abstract) base class for both the cache (as the owner)
// and the worker.
class CachingReaderChunk {
public:
    static const SINT kInvalidIndex;
    static const SINT kChannels;
    static const SINT kFrames;
    static const SINT kSamples;

    // Returns the corresponding chunk index for a frame index
    inline static SINT indexForFrame(SINT frameIndex) {
        DEBUG_ASSERT(mixxx::AudioSource::getMinFrameIndex() <= frameIndex);
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

    // Disable copy and move constructors
    CachingReaderChunk(const CachingReaderChunk&) = delete;
    CachingReaderChunk(CachingReaderChunk&&) = delete;

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
            const mixxx::AudioSourcePointer& pAudioSource,
            SINT maxReadableFrameIndex) const;

    // Read sample frames from the audio source and return the
    // number of frames that have been read. The in/out parameter
    // pMaxReadableFrameIndex is adjusted if reading fails.
    SINT readSampleFrames(
            const mixxx::AudioSourcePointer& pAudioSource,
            SINT* pMaxReadableFrameIndex);

    // Copy sampleCount samples starting at sampleOffset from
    // the chunk's internal buffer into sampleBuffer.
    void copySamples(
            CSAMPLE* sampleBuffer,
            SINT sampleOffset,
            SINT sampleCount) const;

    // Copy sampleCount samples in reverse order starting at sampleOffset from
    // the chunk's internal buffer into sampleBuffer.
    void copySamplesReverse(
            CSAMPLE* sampleBuffer,
            SINT sampleOffset,
            SINT sampleCount) const;

protected:
    explicit CachingReaderChunk(CSAMPLE* sampleBuffer);
    virtual ~CachingReaderChunk();

    void init(SINT index);

private:
    volatile SINT m_index;

    // The worker thread will fill the sample buffer and
    // set the frame count.
    CSAMPLE* const m_sampleBuffer;
    volatile SINT m_frameCount;
};

// This derived class is only accessible for the cache as the owner,
// but not the worker thread. The state READ_PENDING indicates that
// the worker thread is in control.
class CachingReaderChunkForOwner: public CachingReaderChunk {
public:
    explicit CachingReaderChunkForOwner(CSAMPLE* sampleBuffer);
    virtual ~CachingReaderChunkForOwner();

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

    // The state is controlled by the cache as the owner of each chunk!
    void giveToWorker() {
        DEBUG_ASSERT(READY == m_state);
        m_state = READ_PENDING;
    }
    void takeFromWorker() {
        DEBUG_ASSERT(READ_PENDING == m_state);
        m_state = READY;
    }

    // Inserts a chunk into the double-linked list before the
    // given chunk. If the list is currently empty simply pass
    // pBefore = nullptr. Please note that if pBefore points to
    // the head of the current list this chunk becomes the new
    // head of the list.
    void insertIntoListBefore(
            CachingReaderChunkForOwner* pBefore);
    // Removes a chunk from the double-linked list and optionally
    // adjusts head/tail pointers. Pass ppHead/ppTail = nullptr if
    // you prefer to adjust those pointers manually.
    void removeFromList(
            CachingReaderChunkForOwner** ppHead,
            CachingReaderChunkForOwner** ppTail);

private:
    State m_state;

    CachingReaderChunkForOwner* m_pPrev; // previous item in double-linked list
    CachingReaderChunkForOwner* m_pNext; // next item in double-linked list
};


#endif // ENGINE_CACHINGREADERCHUNK_H
