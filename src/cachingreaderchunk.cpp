#include <QtDebug>

#include "cachingreaderchunk.h"

#include "sampleutil.h"
#include "util/math.h"


const SINT CachingReaderChunkForWorker::kInvalidIndex = -1;

// One chunk should contain 1/2 - 1/4th of a second of audio.
// 8192 frames contain about 170 ms of audio at 48 kHz, which
// is well above (hopefully) the latencies people are seeing.
// At 10 ms latency one chunk is enough for 17 callbacks.
// Additionally the chunk size should be a power of 2 for
// easier memory alignment.
// TODO(XXX): The optimum value of the "constant" kFrames depends
// on the properties of the AudioSource as the remarks above suggest!
const SINT CachingReaderChunkForWorker::kChannels = Mixxx::AudioSource::kChannelCountStereo;
const SINT CachingReaderChunkForWorker::kFrames = 8192; // ~ 170 ms at 48 kHz
const SINT CachingReaderChunkForWorker::kSamples =
        CachingReaderChunkForWorker::frames2samples(CachingReaderChunkForWorker::kFrames);

CachingReaderChunkForWorker::CachingReaderChunkForWorker(
        CSAMPLE* sampleBuffer)
        : m_index(kInvalidIndex),
          m_sampleBuffer(sampleBuffer),
          m_frameCount(0) {
}

CachingReaderChunkForWorker::~CachingReaderChunkForWorker() {
}

void CachingReaderChunkForWorker::init(SINT index) {
    m_index = index;
    m_frameCount = 0;
}

bool CachingReaderChunkForWorker::isReadable(
        const Mixxx::AudioSourcePointer& pAudioSource,
        SINT maxReadableFrameIndex) const {
    DEBUG_ASSERT(Mixxx::AudioSource::getMinFrameIndex() <= maxReadableFrameIndex);

    if (!isValid() || pAudioSource.isNull()) {
        return false;
    }
    const SINT frameIndex = frameForIndex(getIndex());
    const SINT maxFrameIndex = math_min(
            maxReadableFrameIndex, pAudioSource->getMaxFrameIndex());
    return frameIndex <= maxFrameIndex;
}

SINT CachingReaderChunkForWorker::readSampleFrames(
        const Mixxx::AudioSourcePointer& pAudioSource,
        SINT* pMaxReadableFrameIndex) {
    DEBUG_ASSERT(pMaxReadableFrameIndex);

    const SINT frameIndex = frameForIndex(getIndex());
    const SINT maxFrameIndex = math_min(
            *pMaxReadableFrameIndex, pAudioSource->getMaxFrameIndex());
    const SINT framesRemaining =
            *pMaxReadableFrameIndex - frameIndex;
    const SINT framesToRead =
            math_min(kFrames, framesRemaining);

    SINT seekFrameIndex =
            pAudioSource->seekSampleFrame(frameIndex);
    if (frameIndex != seekFrameIndex) {
        // Failed to seek to the requested index. The file might
        // be corrupt and decoding should be aborted.
        qWarning() << "Failed to seek chunk position:"
                << "actual =" << seekFrameIndex
                << ", expected =" << frameIndex
                << ", maximum =" << maxFrameIndex;
        if (frameIndex >= seekFrameIndex) {
            // Simple strategy to compensate for seek inaccuracies in
            // faulty files: Try to skip some samples up to the requested
            // seek position. But only skip twice as many frames/samples
            // as have been requested to avoid decoding great portions of
            // the file for small read requests on seek errors.
            const SINT framesToSkip = frameIndex - seekFrameIndex;
            if (framesToSkip <= (2 * framesToRead)) {
                seekFrameIndex += pAudioSource->skipSampleFrames(framesToSkip);
            }
        }
        if (frameIndex != seekFrameIndex) {
            // Unexpected/premature end of file -> prevent further
            // seeks beyond the current seek position
            *pMaxReadableFrameIndex = math_min(seekFrameIndex, *pMaxReadableFrameIndex);
            // Don't read any samples on a seek failure!
            m_frameCount = 0;
            return m_frameCount;
        }
    }

    DEBUG_ASSERT(frameIndex == seekFrameIndex);
    DEBUG_ASSERT(CachingReaderChunk::kChannels
            == Mixxx::AudioSource::kChannelCountStereo);
    m_frameCount = pAudioSource->readSampleFramesStereo(
            framesToRead, m_sampleBuffer, kSamples);
    if (m_frameCount < framesToRead) {
        qWarning() << "Failed to read chunk samples:"
                << "actual =" << m_frameCount
                << ", expected =" << framesToRead;
        // Adjust the max. readable frame index for future
        // read requests to avoid repeated invalid reads.
        *pMaxReadableFrameIndex = frameIndex + m_frameCount;
    }

    return m_frameCount;
}

void CachingReaderChunkForWorker::copySamples(
        CSAMPLE* sampleBuffer, SINT sampleOffset, SINT sampleCount) const {
    DEBUG_ASSERT(0 <= sampleOffset);
    DEBUG_ASSERT(0 <= sampleCount);
    DEBUG_ASSERT((sampleOffset + sampleCount) <= frames2samples(m_frameCount));
    SampleUtil::copy(sampleBuffer, m_sampleBuffer + sampleOffset, sampleCount);
}

CachingReaderChunk::CachingReaderChunk(
        CSAMPLE* sampleBuffer)
        : CachingReaderChunkForWorker(sampleBuffer),
          m_state(FREE),
          m_pPrev(nullptr),
          m_pNext(nullptr) {
}

CachingReaderChunk::~CachingReaderChunk() {
}

void CachingReaderChunk::init(SINT index) {
    DEBUG_ASSERT(READ_PENDING != m_state);
    CachingReaderChunkForWorker::init(index);
    m_state = READY;
}

void CachingReaderChunk::free() {
    DEBUG_ASSERT(READ_PENDING != m_state);
    CachingReaderChunkForWorker::init(kInvalidIndex);
    m_state = FREE;
}

void CachingReaderChunk::insertIntoListBefore(
        CachingReaderChunk* pBefore) {
    DEBUG_ASSERT(nullptr == m_pNext);
    DEBUG_ASSERT(nullptr == m_pPrev);

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

void CachingReaderChunk::removeFromList(
        CachingReaderChunk** ppHead,
        CachingReaderChunk** ppTail) {
    // Remove this chunk from the double-linked list...
    CachingReaderChunk* pNext = m_pNext;
    CachingReaderChunk* pPrev = m_pPrev;
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
