#include <QtDebug>
#include <QFileInfo>

#include "engine/cachingreader/cachingreader.h"
#include "control/controlobject.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/counter.h"
#include "util/math.h"
#include "util/sample.h"
#include "util/logger.h"


namespace {

mixxx::Logger kLogger("CachingReader");

// This is the default hint frameCount that is adopted in case of Hint::kFrameCountForward and
// Hint::kFrameCountBackward count is provided. It matches 23 ms @ 44.1 kHz
// TODO() Do we suffer cache misses if we use an audio buffer of above 23 ms?
const SINT kDefaultHintFrames = 1024;

// currently CachingReaderWorker::kCachingReaderChunkLength is 65536 (0x10000);
// For 80 chunks we need 5242880 (0x500000) bytes (5 MiB) of Memory
//static
const SINT kNumberOfCachedChunksInMemory = 80;

} // anonymous namespace


CachingReader::CachingReader(QString group,
                             UserSettingsPointer config)
        : m_pConfig(config),
          m_chunkReadRequestFIFO(1024),
          m_readerStatusFIFO(1024),
          m_readerStatus(INVALID),
          m_mruCachingReaderChunk(nullptr),
          m_lruCachingReaderChunk(nullptr),
          m_sampleBuffer(CachingReaderChunk::kSamples * kNumberOfCachedChunksInMemory),
          m_worker(group, &m_chunkReadRequestFIFO, &m_readerStatusFIFO) {

    m_allocatedCachingReaderChunks.reserve(kNumberOfCachedChunksInMemory);
    // Divide up the allocated raw memory buffer into total_chunks
    // chunks. Initialize each chunk to hold nothing and add it to the free
    // list.
    for (SINT i = 0; i < kNumberOfCachedChunksInMemory; ++i) {
        CachingReaderChunkForOwner* c =
                new CachingReaderChunkForOwner(
                        mixxx::SampleBuffer::WritableSlice(
                                m_sampleBuffer,
                                CachingReaderChunk::kSamples * i,
                                CachingReaderChunk::kSamples));
        m_chunks.push_back(c);
        m_freeChunks.push_back(c);
    }

    // Forward signals from worker
    connect(&m_worker, &CachingReaderWorker::trackLoading,
            this, &CachingReader::trackLoading,
            Qt::DirectConnection);
    connect(&m_worker, &CachingReaderWorker::trackLoaded,
            this, &CachingReader::trackLoaded,
            Qt::DirectConnection);
    connect(&m_worker, &CachingReaderWorker::trackLoadFailed,
            this, &CachingReader::trackLoadFailed,
            Qt::DirectConnection);

    m_worker.start(QThread::HighPriority);
}

CachingReader::~CachingReader() {
    m_worker.quitWait();
    qDeleteAll(m_chunks);
}

void CachingReader::freeChunk(CachingReaderChunkForOwner* pChunk) {
    DEBUG_ASSERT(pChunk != nullptr);
    DEBUG_ASSERT(pChunk->getState() != CachingReaderChunkForOwner::READ_PENDING);

    const int removed = m_allocatedCachingReaderChunks.remove(pChunk->getIndex());
    // We'll tolerate not being in allocatedCachingReaderChunks,
    // because sometime you free a chunk right after you allocated it.
    DEBUG_ASSERT(removed <= 1);

    pChunk->removeFromList(
            &m_mruCachingReaderChunk, &m_lruCachingReaderChunk);
    pChunk->free();
    m_freeChunks.push_back(pChunk);
}

void CachingReader::freeAllChunks() {
    for (const auto& pChunk: qAsConst(m_chunks)) {
        // We will receive CHUNK_READ_INVALID for all pending chunk reads
        // which should free the chunks individually.
        if (pChunk->getState() == CachingReaderChunkForOwner::READ_PENDING) {
            continue;
        }

        if (pChunk->getState() != CachingReaderChunkForOwner::FREE) {
            pChunk->removeFromList(
                    &m_mruCachingReaderChunk, &m_lruCachingReaderChunk);
            pChunk->free();
            m_freeChunks.push_back(pChunk);
        }
    }

    m_allocatedCachingReaderChunks.clear();
    m_mruCachingReaderChunk = nullptr;
}

CachingReaderChunkForOwner* CachingReader::allocateChunk(SINT chunkIndex) {
    if (m_freeChunks.isEmpty()) {
        return nullptr;
    }
    CachingReaderChunkForOwner* pChunk = m_freeChunks.takeFirst();
    pChunk->init(chunkIndex);

    //kLogger.debug() << "Allocating chunk" << pChunk << pChunk->getIndex();
    m_allocatedCachingReaderChunks.insert(chunkIndex, pChunk);

    return pChunk;
}

CachingReaderChunkForOwner* CachingReader::allocateChunkExpireLRU(SINT chunkIndex) {
    CachingReaderChunkForOwner* pChunk = allocateChunk(chunkIndex);
    if (!pChunk) {
        if (m_lruCachingReaderChunk == nullptr) {
            kLogger.warning() << "ERROR: No LRU chunk to free in allocateChunkExpireLRU.";
            return nullptr;
        }
        freeChunk(m_lruCachingReaderChunk);
        pChunk = allocateChunk(chunkIndex);
    }
    //kLogger.debug() << "allocateChunkExpireLRU" << chunk << pChunk;
    return pChunk;
}

CachingReaderChunkForOwner* CachingReader::lookupChunk(SINT chunkIndex) {
    // Defaults to nullptr if it's not in the hash.
    CachingReaderChunkForOwner* chunk = m_allocatedCachingReaderChunks.value(chunkIndex, nullptr);

    // Make sure the allocated number matches the indexed chunk number.
    DEBUG_ASSERT(chunk == nullptr || chunkIndex == chunk->getIndex());

    return chunk;
}

void CachingReader::freshenChunk(CachingReaderChunkForOwner* pChunk) {
    DEBUG_ASSERT(pChunk != nullptr);
    DEBUG_ASSERT(pChunk->getState() != CachingReaderChunkForOwner::READ_PENDING);

    // Remove the chunk from the LRU list
    pChunk->removeFromList(&m_mruCachingReaderChunk, &m_lruCachingReaderChunk);

    // Adjust the least-recently-used item before inserting the
    // chunk as the new most-recently-used item.
    if (m_lruCachingReaderChunk == nullptr) {
        if (m_mruCachingReaderChunk == nullptr) {
            m_lruCachingReaderChunk = pChunk;
        } else {
            m_lruCachingReaderChunk = m_mruCachingReaderChunk;
        }
    }

    // Insert the chunk as the new most-recently-used item.
    pChunk->insertIntoListBefore(m_mruCachingReaderChunk);
    m_mruCachingReaderChunk = pChunk;
}

CachingReaderChunkForOwner* CachingReader::lookupChunkAndFreshen(SINT chunkIndex) {
    CachingReaderChunkForOwner* pChunk = lookupChunk(chunkIndex);
    if (pChunk &&
            (pChunk->getState() != CachingReaderChunkForOwner::READ_PENDING)) {
        freshenChunk(pChunk);
    }
    return pChunk;
}

void CachingReader::newTrack(TrackPointer pTrack) {
    m_worker.newTrack(pTrack);
    m_worker.workReady();
}

void CachingReader::process() {
    ReaderStatusUpdate status;
    while (m_readerStatusFIFO.read(&status, 1) == 1) {
        CachingReaderChunkForOwner* pChunk = static_cast<CachingReaderChunkForOwner*>(status.chunk);
        if (pChunk) {
            // Take over control of the chunk from the worker.
            // This has to be done before freeing all chunks
            // after a new track has been loaded (see below)!
            pChunk->takeFromWorker();
            if (status.status == CHUNK_READ_SUCCESS) {
                // Insert or freshen the chunk in the MRU/LRU list after
                // obtaining ownership from the worker.
                freshenChunk(pChunk);
            } else {
                // Discard chunks that don't carry any data
                freeChunk(pChunk);
            }
        }
        if (status.status == TRACK_NOT_LOADED) {
            m_readerStatus = status.status;
        } else if (status.status == TRACK_LOADED) {
            m_readerStatus = status.status;
            // Reset the max. readable frame index
            m_readableFrameIndexRange = status.readableFrameIndexRange();
            // Free all chunks with sample data from a previous track
            freeAllChunks();
        }
        if (m_readerStatus == TRACK_LOADED) {
            // Adjust the readable frame index range after loading or reading
            m_readableFrameIndexRange = intersect(
                    m_readableFrameIndexRange,
                    status.readableFrameIndexRange());
        } else {
            // Reset the readable frame index range
            m_readableFrameIndexRange = mixxx::IndexRange();
        }
    }
}

CachingReader::ReadResult CachingReader::read(SINT startSample, SINT numSamples, bool reverse, CSAMPLE* buffer) {
    // Check for bad inputs
    VERIFY_OR_DEBUG_ASSERT(
            // Refuse to read from an invalid position
            (startSample % CachingReaderChunk::kChannels == 0) &&
            // Refuse to read from an invalid number of samples
            (numSamples % CachingReaderChunk::kChannels == 0) && (numSamples >= 0)) {
        kLogger.critical()
                << "read() invalid arguments:"
                << "startSample =" << startSample
                << "numSamples =" << numSamples;
        return ReadResult::UNAVAILABLE;
    }
    VERIFY_OR_DEBUG_ASSERT(buffer) {
        return ReadResult::UNAVAILABLE;
    }

    // If no track is loaded, don't do anything.
    if (m_readerStatus != TRACK_LOADED) {
        return ReadResult::UNAVAILABLE;
    }

    // If asked to read 0 samples, don't do anything. (this is a perfectly
    // reasonable request that happens sometimes.
    if (numSamples == 0) {
        return ReadResult::AVAILABLE; // nothing to do
    }

    // the samples are always read in forward direction
    // If reverse = true, the frames are copied in reverse order to the
    // destination buffer
    SINT sample = startSample;
    if (reverse) {
        // Start with the last sample in buffer
        sample -= numSamples;
    }

    SINT samplesRemaining = numSamples;

    // Process new messages from the reader thread before looking up
    // the first chunk and to update m_readableFrameIndexRange
    process();

    auto remainingFrameIndexRange =
            mixxx::IndexRange::forward(
                    CachingReaderChunk::samples2frames(sample),
                    CachingReaderChunk::samples2frames(numSamples));
    DEBUG_ASSERT(!remainingFrameIndexRange.empty());

    auto result = ReadResult::AVAILABLE;
    if (!intersect(remainingFrameIndexRange, m_readableFrameIndexRange).empty()) {
        // Fill the buffer up to the first readable sample with
        // silence. This may happen when the engine is in preroll,
        // i.e. if the frame index points a region before the first
        // track sample.
        if (remainingFrameIndexRange.start() < m_readableFrameIndexRange.start()) {
            const auto prerollFrameIndexRange =
                    mixxx::IndexRange::between(
                            remainingFrameIndexRange.start(),
                            m_readableFrameIndexRange.start());
            DEBUG_ASSERT(prerollFrameIndexRange.length() <= remainingFrameIndexRange.length());
            kLogger.debug()
                    << "Prepending"
                    << prerollFrameIndexRange.length()
                    << "frames of silence";
            const SINT prerollFrames = prerollFrameIndexRange.length();
            const SINT prerollSamples = CachingReaderChunk::frames2samples(prerollFrames);
            DEBUG_ASSERT(samplesRemaining >= prerollSamples);
            if (reverse) {
                SampleUtil::clear(&buffer[samplesRemaining - prerollSamples], prerollSamples);
            } else {
                SampleUtil::clear(buffer, prerollSamples);
                buffer += prerollSamples;
            }
            samplesRemaining -= prerollSamples;
            remainingFrameIndexRange.shrinkFront(prerollFrames);
            result = ReadResult::PARTIALLY_AVAILABLE;
        }

        // Read the actual samples from the audio source into the
        // buffer. The buffer will be filled with silence for every
        // unreadable sample or samples outside of the track region
        // later at the end of this function.
        if (!remainingFrameIndexRange.empty()) {
            // The intersection between the readable samples from the track
            // and the requested samples is not empty, so start reading.
            DEBUG_ASSERT(!intersect(remainingFrameIndexRange, m_readableFrameIndexRange).empty());
            DEBUG_ASSERT(remainingFrameIndexRange.start() >= m_readableFrameIndexRange.start());

            const SINT firstChunkIndex =
                    CachingReaderChunk::indexForFrame(remainingFrameIndexRange.start());
            SINT lastChunkIndex =
                    CachingReaderChunk::indexForFrame(remainingFrameIndexRange.end() - 1);
            for (SINT chunkIndex = firstChunkIndex;
                    chunkIndex <= lastChunkIndex;
                    ++chunkIndex) {

                // Process new messages from the reader thread before looking up
                // the next chunk
                process();

                // m_readableFrameIndexRange might change with every read operation!
                // On a cache miss audio data will be read from the audio source in
                // process() and the readable frame index range might get adjusted
                // if decoding errors occur.
                remainingFrameIndexRange =
                        intersect(
                                remainingFrameIndexRange,
                                m_readableFrameIndexRange);

                if (remainingFrameIndexRange.empty()) {
                    // No more readable data available. Exit the loop and
                    // fill the remaining buffer with silence.
                    kLogger.warning() << "Failed to read more sample data";
                    break;
                }
                lastChunkIndex =
                        CachingReaderChunk::indexForFrame(remainingFrameIndexRange.end() - 1);
                if (lastChunkIndex < chunkIndex) {
                    // No more readable data available. Exit the loop and
                    // fill the remaining buffer with silence.
                    kLogger.warning() << "Abort reading of sample data";
                    break;
                }

                mixxx::IndexRange bufferedFrameIndexRange;
                const CachingReaderChunkForOwner* const pChunk = lookupChunkAndFreshen(chunkIndex);
                if (pChunk && (pChunk->getState() == CachingReaderChunkForOwner::READY)) {
                    if (reverse) {
                        bufferedFrameIndexRange =
                                pChunk->readBufferedSampleFramesReverse(
                                        &buffer[samplesRemaining],
                                        remainingFrameIndexRange);
                    } else {
                        bufferedFrameIndexRange =
                                pChunk->readBufferedSampleFrames(
                                        buffer,
                                        remainingFrameIndexRange);
                    }
                } else {
                    // This will happen regularly when jumping to a new position
                    // within the file and decoding of the audio data is still
                    // pending.
                    DEBUG_ASSERT(!pChunk ||
                            (pChunk->getState() == CachingReaderChunkForOwner::READ_PENDING));
                    Counter("CachingReader::read(): Failed to read chunk on cache miss")++;
                    if (kLogger.traceEnabled()) {
                        kLogger.trace()
                                << "Cache miss for chunk with index"
                                << chunkIndex
                                << "- abort reading";
                    }
                    // Abort reading (see below)
                    DEBUG_ASSERT(bufferedFrameIndexRange.empty());
                }
                if (bufferedFrameIndexRange.empty()) {
                    if (samplesRemaining == numSamples) {
                        DEBUG_ASSERT(chunkIndex == firstChunkIndex);
                        // We have not read a single frame caused by a cache miss of
                        // the first required chunk. Inform the calling code that no
                        // data has been written into the buffer and to handle this
                        // situation appropriately.
                        return ReadResult::UNAVAILABLE;
                    }
                    // No more readable data available. Exit the loop and
                    // finally fill the remaining buffer with silence.
                    break;
                }
                DEBUG_ASSERT(bufferedFrameIndexRange <= remainingFrameIndexRange);
                if (remainingFrameIndexRange.start() < bufferedFrameIndexRange.start()) {
                    const auto paddingFrameIndexRange =
                            mixxx::IndexRange::between(
                                    remainingFrameIndexRange.start(),
                                    bufferedFrameIndexRange.start());
                    kLogger.warning()
                            << "Inserting"
                            << paddingFrameIndexRange.length()
                            << "frames of silence for unreadable audio data";
                    SINT paddingSamples = CachingReaderChunk::frames2samples(paddingFrameIndexRange.length());
                    DEBUG_ASSERT(samplesRemaining >= paddingSamples);
                    if (reverse) {
                        SampleUtil::clear(&buffer[samplesRemaining - paddingSamples], paddingSamples);
                    } else {
                        SampleUtil::clear(buffer, paddingSamples);
                        buffer += paddingSamples;
                    }
                    samplesRemaining -= paddingSamples;
                    remainingFrameIndexRange.shrinkFront(paddingFrameIndexRange.length());
                    result = ReadResult::PARTIALLY_AVAILABLE;
                }
                const SINT chunkSamples =
                        CachingReaderChunk::frames2samples(bufferedFrameIndexRange.length());
                DEBUG_ASSERT(chunkSamples > 0);
                if (!reverse) {
                    buffer += chunkSamples;
                }
                DEBUG_ASSERT(samplesRemaining >= chunkSamples);
                samplesRemaining -= chunkSamples;
                remainingFrameIndexRange.shrinkFront(bufferedFrameIndexRange.length());
            }
        }
    }
    // Finally fill the remaining buffer with silence
    DEBUG_ASSERT(samplesRemaining >= 0);
    if (samplesRemaining > 0) {
        SampleUtil::clear(buffer, samplesRemaining);
        result = ReadResult::PARTIALLY_AVAILABLE;
    }
    return result;
}

void CachingReader::hintAndMaybeWake(const HintVector& hintList) {
    // If no file is loaded, skip.
    if (m_readerStatus != TRACK_LOADED) {
        return;
    }

    // For every chunk that the hints indicated, check if it is in the cache. If
    // any are not, then wake.
    bool shouldWake = false;

    for (const auto& hint: hintList) {
        SINT hintFrame = hint.frame;
        SINT hintFrameCount = hint.frameCount;

        // Handle some special length values
        if (hintFrameCount == Hint::kFrameCountForward) {
        	hintFrameCount = kDefaultHintFrames;
        } else if (hintFrameCount == Hint::kFrameCountBackward) {
        	hintFrame -= kDefaultHintFrames;
        	hintFrameCount = kDefaultHintFrames;
            if (hintFrame < 0) {
            	hintFrameCount += hintFrame;
                hintFrame = 0;
            }
        }

        VERIFY_OR_DEBUG_ASSERT(hintFrameCount > 0) {
            kLogger.warning() << "ERROR: Negative hint length. Ignoring.";
            continue;
        }

        const auto readableFrameIndexRange = intersect(
                m_readableFrameIndexRange,
                mixxx::IndexRange::forward(hintFrame, hintFrameCount));
        if (readableFrameIndexRange.empty()) {
            continue;
        }

        const int firstChunkIndex = CachingReaderChunk::indexForFrame(readableFrameIndexRange.start());
        const int lastChunkIndex = CachingReaderChunk::indexForFrame(readableFrameIndexRange.end() - 1);
        for (int chunkIndex = firstChunkIndex; chunkIndex <= lastChunkIndex; ++chunkIndex) {
            CachingReaderChunkForOwner* pChunk = lookupChunk(chunkIndex);
            if (pChunk == nullptr) {
                shouldWake = true;
                pChunk = allocateChunkExpireLRU(chunkIndex);
                if (pChunk == nullptr) {
                    kLogger.warning() << "ERROR: Couldn't allocate spare CachingReaderChunk to make CachingReaderChunkReadRequest.";
                    continue;
                }
                // Do not insert the allocated chunk into the MRU/LRU list,
                // because it will be handed over to the worker immediately
                CachingReaderChunkReadRequest request;
                request.giveToWorker(pChunk);
                // kLogger.debug() << "Requesting read of chunk" << current << "into" << pChunk;
                // kLogger.debug() << "Requesting read into " << request.chunk->data;
                if (m_chunkReadRequestFIFO.write(&request, 1) != 1) {
                    kLogger.warning() << "ERROR: Could not submit read request for "
                             << chunkIndex;
                    // Revoke the chunk from the worker and free it
                    pChunk->takeFromWorker();
                    freeChunk(pChunk);
                }
                //kLogger.debug() << "Checking chunk " << current << " shouldWake:" << shouldWake << " chunksToRead" << m_chunksToRead.size();
            } else if (pChunk->getState() == CachingReaderChunkForOwner::READY) {
                // This will cause the chunk to be 'freshened' in the cache. The
                // chunk will be moved to the end of the LRU list.
                freshenChunk(pChunk);
            }
        }
    }

    // If there are chunks to be read, wake up.
    if (shouldWake) {
        m_worker.workReady();
    }
}
