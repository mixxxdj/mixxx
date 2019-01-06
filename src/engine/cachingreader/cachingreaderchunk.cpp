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
    DEBUG_ASSERT(READ_PENDING != m_state);
    CachingReaderChunk::init(index);
    m_state = READY;
}

void CachingReaderChunkForOwner::free() {
    DEBUG_ASSERT(READ_PENDING != m_state);
    CachingReaderChunk::init(kInvalidChunkIndex);
    m_state = FREE;
}

void CachingReaderChunkForOwner::insertIntoListBefore(
        CachingReaderChunkForOwner* pBefore) {
    DEBUG_ASSERT(m_pNext == nullptr);
    DEBUG_ASSERT(m_pPrev == nullptr);
    DEBUG_ASSERT(m_state != READ_PENDING); // Must not be accessed by a worker!

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
    // Remove this chunk from the double-linked list...
    CachingReaderChunkForOwner* pNext = m_pNext;
    CachingReaderChunkForOwner* pPrev = m_pPrev;
    m_pNext = nullptr;
    m_pPrev = nullptr;

    // ...reconnect the remaining list elements...
    if (pNext) {
        DEBUG_ASSERT(this == pNext->m_pPrev);
        pNext->m_pPrev = pPrev;
    }
    if (pPrev) {
        DEBUG_ASSERT(this == pPrev->m_pNext);
        pPrev->m_pNext = pNext;
    }

    // ...and adjust head/tail.
    if (ppHead && (this == *ppHead)) {
        *ppHead = pNext;
    }
    if (ppTail && (this == *ppTail)) {
        *ppTail = pPrev;
    }
}
