#pragma once

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
  // One chunk should contain 1/2 - 1/4th of a second of audio.
  // 8192 frames contain about 170 ms of audio at 48 kHz, which
  // is well above (hopefully) the latencies people are seeing.
  // At 10 ms latency one chunk is enough for 17 callbacks.
  // Additionally the chunk size should be a power of 2 for
  // easier memory alignment.
  // TODO(XXX): The optimum value of the "constant" kFrames depends
  // on the properties of the AudioSource as the remarks above suggest!
  static constexpr mixxx::audio::ChannelCount kChannels = mixxx::kEngineChannelCount;
  static constexpr SINT kFrames = 8192; // ~ 170 ms at 48 kHz
  static constexpr SINT kSamples = kFrames * kChannels;

  // Converts frames to samples
  static constexpr SINT frames2samples(SINT frames) noexcept {
      return frames * kChannels;
  }
  static constexpr double dFrames2samples(SINT frames) noexcept {
      return static_cast<double>(frames) * kChannels;
  }
    // Converts samples to frames
    static SINT samples2frames(SINT samples) {
        DEBUG_ASSERT(0 == (samples % kChannels));
        return samples / kChannels;
    }

    // Returns the corresponding chunk index for a frame index
    static SINT indexForFrame(
            /*const mixxx::AudioSourcePointer& pAudioSource,*/
            SINT frameIndex) {
        // DEBUG_ASSERT(pAudioSource->frameIndexRange().contains(frameIndex));
        const SINT frameIndexOffset = frameIndex /*- pAudioSource->frameIndexMin()*/;
        return frameIndexOffset / kFrames;
    }

    // Disable copy and move constructors
    CachingReaderChunk(const CachingReaderChunk&) = delete;
    CachingReaderChunk(CachingReaderChunk&&) = delete;

    SINT getIndex() const noexcept {
        return m_index;
    }

    // Frame index range of this chunk for the given audio source.
    mixxx::IndexRange frameIndexRange(
            const mixxx::AudioSourcePointer& pAudioSource) const;

    // Read sample frames from the audio source and return the
    // range of frames that have been read.
    mixxx::IndexRange bufferSampleFrames(
            const mixxx::AudioSourcePointer& pAudioSource,
            mixxx::SampleBuffer::WritableSlice tempOutputBuffer);

    mixxx::IndexRange readBufferedSampleFrames(
            CSAMPLE* sampleBuffer,
            const mixxx::IndexRange& frameIndexRange) const;
    mixxx::IndexRange readBufferedSampleFramesReverse(
            CSAMPLE* reverseSampleBuffer,
            const mixxx::IndexRange& frameIndexRange) const;

protected:
    explicit CachingReaderChunk(
            mixxx::SampleBuffer::WritableSlice sampleBuffer);
    virtual ~CachingReaderChunk() = default;

    void init(SINT index);

private:
  SINT frameIndexOffset() const noexcept {
        return m_index * kFrames;
  }

    SINT m_index;

    // The worker thread will fill the sample buffer and
    // set the corresponding frame index range.
    mixxx::SampleBuffer::WritableSlice m_sampleBuffer;
    mixxx::ReadableSampleFrames m_bufferedSampleFrames;
};

// This derived class is only accessible for the cache as the owner,
// but not the worker thread. The state READ_PENDING indicates that
// the worker thread is in control.
class CachingReaderChunkForOwner: public CachingReaderChunk {
public:
    explicit CachingReaderChunkForOwner(
            mixxx::SampleBuffer::WritableSlice sampleBuffer);
    ~CachingReaderChunkForOwner() override = default;

    void init(SINT index);
    void free();

    enum State {
        FREE,
        READY,
        READ_PENDING
    };

    State getState() const noexcept {
        return m_state;
    }

    // The state is controlled by the cache as the owner of each chunk!
    void giveToWorker() {
        // Must not be referenced in MRU/LRU list!
        DEBUG_ASSERT(!m_pPrev);
        DEBUG_ASSERT(!m_pNext);
        DEBUG_ASSERT(m_state == READY);
        m_state = READ_PENDING;
    }
    void takeFromWorker() {
        // Must not be referenced in MRU/LRU list!
        DEBUG_ASSERT(!m_pPrev);
        DEBUG_ASSERT(!m_pNext);
        DEBUG_ASSERT(m_state == READ_PENDING);
        m_state = READY;
    }

    // Inserts a chunk into the double-linked list before the
    // given chunk and adjusts the head/tail pointers. The
    // chunk is inserted at the tail of the list if
    // pBefore == nullptr.
    void insertIntoListBefore(
            CachingReaderChunkForOwner** ppHead,
            CachingReaderChunkForOwner** ppTail,
            CachingReaderChunkForOwner* pBefore);
    // Removes a chunk from the double-linked list and adjusts
    // the head/tail pointers.
    void removeFromList(
            CachingReaderChunkForOwner** ppHead,
            CachingReaderChunkForOwner** ppTail);

private:
  State m_state;

  CachingReaderChunkForOwner* m_pPrev; // previous item in double-linked list
  CachingReaderChunkForOwner* m_pNext; // next item in double-linked list
};
