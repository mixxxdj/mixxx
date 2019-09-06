#include "engine/cachingreaderchunk.h"

#include <QtDebug>

#include "sources/audiosourcestereoproxy.h"
#include "engine/engine.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/logger.h"


namespace {

mixxx::Logger kLogger("CachingReaderChunk");

const SINT kInvalidChunkIndex = -1;

} // anonymous namespace

// One chunk should contain 1/2 - 1/4th of a second of audio.
// 8192 frames contain about 170 ms of audio at 48 kHz, which
// is well above (hopefully) the latencies people are seeing.
// At 10 ms latency one chunk is enough for 17 callbacks.
// Additionally the chunk size should be a power of 2 for
// easier memory alignment.
// TODO(XXX): The optimum value of the "constant" kFrames depends
// on the properties of the AudioSource as the remarks above suggest!
const mixxx::AudioSignal::ChannelCount CachingReaderChunk::kChannels = mixxx::kEngineChannelCount;
const SINT CachingReaderChunk::kFrames = 8192; // ~ 170 ms at 48 kHz
const SINT CachingReaderChunk::kSamples =
        CachingReaderChunk::frames2samples(CachingReaderChunk::kFrames);

CachingReaderChunk::CachingReaderChunk(
        mixxx::SampleBuffer::WritableSlice sampleBuffer)
        : m_index(kInvalidChunkIndex),
          m_sampleBuffer(sampleBuffer) {
    DEBUG_ASSERT(sampleBuffer.length() == kSamples);
}

CachingReaderChunk::~CachingReaderChunk() {
}

void CachingReaderChunk::init(SINT index) {
    m_index = index;
    m_bufferedSampleFrames.frameIndexRange() = mixxx::IndexRange();
}

// Frame index range of this chunk for the given audio source.
mixxx::IndexRange CachingReaderChunk::frameIndexRange(
        const mixxx::AudioSourcePointer& pAudioSource) const {
    if (!pAudioSource) {
        return mixxx::IndexRange();
    }
    const SINT minFrameIndex =
            pAudioSource->frameIndexMin() +
            frameIndexOffset();
    return intersect(
            mixxx::IndexRange::forward(minFrameIndex, kFrames),
            pAudioSource->frameIndexRange());
}

mixxx::IndexRange CachingReaderChunk::bufferSampleFrames(
        const mixxx::AudioSourcePointer& pAudioSource,
        mixxx::SampleBuffer::WritableSlice tempOutputBuffer) {
    const auto sourceFrameIndexRange = frameIndexRange(pAudioSource);
    mixxx::AudioSourceStereoProxy audioSourceProxy(
            pAudioSource,
            tempOutputBuffer);
    DEBUG_ASSERT(audioSourceProxy.channelCount() == kChannels);
    m_bufferedSampleFrames =
            audioSourceProxy.readSampleFrames(
                    mixxx::WritableSampleFrames(
                            sourceFrameIndexRange,
                            mixxx::SampleBuffer::WritableSlice(m_sampleBuffer)));
    DEBUG_ASSERT(m_bufferedSampleFrames.frameIndexRange() <= sourceFrameIndexRange);
    return m_bufferedSampleFrames.frameIndexRange();
}

mixxx::IndexRange CachingReaderChunk::readBufferedSampleFrames(
        CSAMPLE* sampleBuffer,
        const mixxx::IndexRange& frameIndexRange) const {
    const auto copyableFrameIndexRange =
            intersect(frameIndexRange, m_bufferedSampleFrames.frameIndexRange());
    if (!copyableFrameIndexRange.empty()) {
        const SINT dstSampleOffset =
                frames2samples(copyableFrameIndexRange.start() - frameIndexRange.start());
        const SINT srcSampleOffset =
                frames2samples(copyableFrameIndexRange.start() - m_bufferedSampleFrames.frameIndexRange().start());
        const SINT sampleCount = frames2samples(copyableFrameIndexRange.length());
        SampleUtil::copy(
                sampleBuffer + dstSampleOffset,
                m_bufferedSampleFrames.readableData(srcSampleOffset),
                sampleCount);
    }
    return copyableFrameIndexRange;
}

mixxx::IndexRange CachingReaderChunk::readBufferedSampleFramesReverse(
        CSAMPLE* reverseSampleBuffer,
        const mixxx::IndexRange& frameIndexRange) const {
    const auto copyableFrameIndexRange =
            intersect(frameIndexRange, m_bufferedSampleFrames.frameIndexRange());
    if (!copyableFrameIndexRange.empty()) {
        const SINT dstSampleOffset =
                frames2samples(copyableFrameIndexRange.start() - frameIndexRange.start());
        const SINT srcSampleOffset =
                frames2samples(copyableFrameIndexRange.start() - m_bufferedSampleFrames.frameIndexRange().start());
        const SINT sampleCount = frames2samples(copyableFrameIndexRange.length());
        SampleUtil::copyReverse(
                reverseSampleBuffer - dstSampleOffset - sampleCount,
                m_bufferedSampleFrames.readableData(srcSampleOffset),
                sampleCount);
    }
    return copyableFrameIndexRange;
}

CachingReaderChunkForOwner::CachingReaderChunkForOwner(
        mixxx::SampleBuffer::WritableSlice sampleBuffer)
        : CachingReaderChunk(sampleBuffer),
          m_state(FREE),
          m_pPrev(nullptr),
          m_pNext(nullptr) {
}

CachingReaderChunkForOwner::~CachingReaderChunkForOwner() {
}

void CachingReaderChunkForOwner::init(SINT index) {
    // Must not be referenced in MRU/LRU list!
    DEBUG_ASSERT(!m_pNext);
    DEBUG_ASSERT(!m_pPrev);
    // Must not be accessed by a worker!
    DEBUG_ASSERT(m_state != READ_PENDING);
    CachingReaderChunk::init(index);
    m_state = READY;
}

void CachingReaderChunkForOwner::free() {
    // Must not be referenced in MRU/LRU list!
    DEBUG_ASSERT(!m_pNext);
    DEBUG_ASSERT(!m_pPrev);
    // Must not be accessed by a worker!
    DEBUG_ASSERT(m_state != READ_PENDING);
    CachingReaderChunk::init(kInvalidChunkIndex);
    m_state = FREE;
}

void CachingReaderChunkForOwner::insertIntoListBefore(
        CachingReaderChunkForOwner* pBefore) {
    // Must not be referenced in MRU/LRU list!
    DEBUG_ASSERT(!m_pNext);
    DEBUG_ASSERT(!m_pPrev);
    // Must not be accessed by a worker!
    DEBUG_ASSERT(m_state != READ_PENDING);

    m_pNext = pBefore;
    if (pBefore) {
        if (pBefore->m_pPrev) {
            m_pPrev = pBefore->m_pPrev;
            DEBUG_ASSERT(m_pPrev->m_pNext == pBefore);
            m_pPrev->m_pNext = this;
        }
        pBefore->m_pPrev = this;
    }
}

void CachingReaderChunkForOwner::removeFromList(
        CachingReaderChunkForOwner** ppHead,
        CachingReaderChunkForOwner** ppTail) {
    DEBUG_ASSERT(ppHead);
    DEBUG_ASSERT(ppTail);
    if (!m_pPrev && !m_pNext) {
        // Not in linked list -> nothing to do
        return;
    }

    // Remove this chunk from the double-linked list...
    const auto pPrev = m_pPrev;
    const auto pNext = m_pNext;
    m_pPrev = nullptr;
    m_pNext = nullptr;

    // ...reconnect the adjacent list items and adjust head/tail
    if (pPrev) {
        DEBUG_ASSERT(this == pPrev->m_pNext);
        pPrev->m_pNext = pNext;
    } else {
        // No predecessor
        DEBUG_ASSERT(this == *ppHead);
        // pNext becomes the new head
        *ppHead = pNext;
    }
    if (pNext) {
        DEBUG_ASSERT(this == pNext->m_pPrev);
        pNext->m_pPrev = pPrev;
    } else {
        // No successor
        DEBUG_ASSERT(this == *ppTail);
        // pPrev becomes the new tail
        *ppTail = pPrev;
    }
}
