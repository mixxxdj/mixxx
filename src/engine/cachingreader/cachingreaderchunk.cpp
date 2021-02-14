#include "engine/cachingreader/cachingreaderchunk.h"

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
const mixxx::audio::ChannelCount CachingReaderChunk::kChannels = mixxx::kEngineChannelCount;
const SINT CachingReaderChunk::kFrames = 8192; // ~ 170 ms at 48 kHz
const SINT CachingReaderChunk::kSamples =
        CachingReaderChunk::frames2samples(CachingReaderChunk::kFrames);

CachingReaderChunk::CachingReaderChunk(
        mixxx::SampleBuffer::WritableSlice sampleBuffer)
        : m_index(kInvalidChunkIndex),
          m_sampleBuffer(std::move(sampleBuffer)) {
    DEBUG_ASSERT(m_sampleBuffer.length() == kSamples);
}

void CachingReaderChunk::init(SINT index) {
    DEBUG_ASSERT(m_index == kInvalidChunkIndex || index == kInvalidChunkIndex);
    m_index = index;
    m_bufferedSampleFrames.frameIndexRange() = mixxx::IndexRange();
}

// Frame index range of this chunk for the given audio source.
mixxx::IndexRange CachingReaderChunk::frameIndexRange(
        const mixxx::AudioSourcePointer& pAudioSource) const {
    DEBUG_ASSERT(m_index != kInvalidChunkIndex);
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
    DEBUG_ASSERT(m_index != kInvalidChunkIndex);
    const auto sourceFrameIndexRange = frameIndexRange(pAudioSource);
    mixxx::AudioSourceStereoProxy audioSourceProxy(
            pAudioSource,
            tempOutputBuffer);
    DEBUG_ASSERT(
            audioSourceProxy.getSignalInfo().getChannelCount() ==
            kChannels);
    m_bufferedSampleFrames =
            audioSourceProxy.readSampleFrames(
                    mixxx::WritableSampleFrames(
                            sourceFrameIndexRange,
                            mixxx::SampleBuffer::WritableSlice(m_sampleBuffer)));
    DEBUG_ASSERT(m_bufferedSampleFrames.frameIndexRange().empty() ||
            m_bufferedSampleFrames.frameIndexRange().isSubrangeOf(sourceFrameIndexRange));
    return m_bufferedSampleFrames.frameIndexRange();
}

mixxx::IndexRange CachingReaderChunk::readBufferedSampleFrames(
        CSAMPLE* sampleBuffer,
        const mixxx::IndexRange& frameIndexRange) const {
    DEBUG_ASSERT(m_index != kInvalidChunkIndex);
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
    DEBUG_ASSERT(m_index != kInvalidChunkIndex);
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
        : CachingReaderChunk(std::move(sampleBuffer)),
          m_state(FREE),
          m_pPrev(nullptr),
          m_pNext(nullptr) {
}

void CachingReaderChunkForOwner::init(SINT index) {
    // Must not be accessed by a worker!
    DEBUG_ASSERT(m_state != READ_PENDING);
    // Must not be referenced in MRU/LRU list!
    DEBUG_ASSERT(!m_pNext);
    DEBUG_ASSERT(!m_pPrev);

    CachingReaderChunk::init(index);
    m_state = READY;
}

void CachingReaderChunkForOwner::free() {
    // Must not be accessed by a worker!
    DEBUG_ASSERT(m_state != READ_PENDING);
    // Must not be referenced in MRU/LRU list!
    DEBUG_ASSERT(!m_pNext);
    DEBUG_ASSERT(!m_pPrev);

    CachingReaderChunk::init(kInvalidChunkIndex);
    m_state = FREE;
}

void CachingReaderChunkForOwner::insertIntoListBefore(
        CachingReaderChunkForOwner** ppHead,
        CachingReaderChunkForOwner** ppTail,
        CachingReaderChunkForOwner* pBefore) {
    DEBUG_ASSERT(m_state == READY);
    // Both head and tail need to be adjusted
    DEBUG_ASSERT(ppHead);
    DEBUG_ASSERT(ppTail);
    // Cannot insert before itself
    DEBUG_ASSERT(this != pBefore);
    // Must not yet be referenced in MRU/LRU list
    DEBUG_ASSERT(this != *ppHead);
    DEBUG_ASSERT(this != *ppTail);
    DEBUG_ASSERT(!m_pNext);
    DEBUG_ASSERT(!m_pPrev);
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "insertIntoListBefore()"
                << this
                << ppHead << *ppHead
                << ppTail << *ppTail
                << pBefore;
    }

    if (pBefore) {
        // List must already contain one or more item, i.e. has both
        // a head and a tail
        DEBUG_ASSERT(*ppHead);
        DEBUG_ASSERT(*ppTail);
        m_pPrev = pBefore->m_pPrev;
        pBefore->m_pPrev = this;
        m_pNext = pBefore;
        if (*ppHead == pBefore) {
            // Replace head
            *ppHead = this;
        }
    } else {
        // Append as new tail
        m_pPrev = *ppTail;
        *ppTail = this;
        if (m_pPrev) {
            m_pPrev->m_pNext = this;
        }
        if (!*ppHead) {
            // Initialize new head if the list was empty before
            *ppHead = this;
        }
    }
}

void CachingReaderChunkForOwner::removeFromList(
        CachingReaderChunkForOwner** ppHead,
        CachingReaderChunkForOwner** ppTail) {
    DEBUG_ASSERT(m_state == READY);
    // Both head and tail need to be adjusted
    DEBUG_ASSERT(ppHead);
    DEBUG_ASSERT(ppTail);
    if (kLogger.traceEnabled()) {
        kLogger.trace()
                << "removeFromList()"
                << this
                << ppHead << *ppHead
                << ppTail << *ppTail;
    }

    // Disconnect this chunk from the double-linked list
    auto* const pPrev = m_pPrev;
    auto* const pNext = m_pNext;
    m_pPrev = nullptr;
    m_pNext = nullptr;

    // Reconnect the adjacent list items and adjust head/tail if needed
    if (pPrev) {
        DEBUG_ASSERT(this == pPrev->m_pNext);
        pPrev->m_pNext = pNext;
    } else {
        // Only the current head item doesn't have a predecessor
        if (this == *ppHead) {
            // pNext becomes the new head
            *ppHead = pNext;
        } else {
            // Item was not part the list and must not have any successor
            DEBUG_ASSERT(!pPrev);
        }
    }
    if (pNext) {
        DEBUG_ASSERT(this == pNext->m_pPrev);
        pNext->m_pPrev = pPrev;
    } else {
        // Only the current tail item doesn't have a successor
        if (this == *ppTail) {
            // pPrev becomes the new tail
            *ppTail = pPrev;
        } else {
            // Item was not part the list and must not have any predecessor
            DEBUG_ASSERT(!pPrev);
        }
    }
}
