#include <QtDebug>
#include <QFileInfo>

#include "engine/cachingreader.h"
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
// TODO() Do we suffer chache misses if we use an audio buffer of above 23 ms?
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
                        SampleBuffer::WritableSlice(
                                m_sampleBuffer,
                                CachingReaderChunk::kSamples * i,
                                CachingReaderChunk::kSamples));
        m_chunks.push_back(c);
        m_freeChunks.push_back(c);
    }

    // Forward signals from worker
    connect(&m_worker, SIGNAL(trackLoading()),
            this, SIGNAL(trackLoading()),
            Qt::DirectConnection);
    connect(&m_worker, SIGNAL(trackLoaded(TrackPointer, int, int)),
            this, SIGNAL(trackLoaded(TrackPointer, int, int)),
            Qt::DirectConnection);
    connect(&m_worker, SIGNAL(trackLoadFailed(TrackPointer, QString)),
            this, SIGNAL(trackLoadFailed(TrackPointer, QString)),
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
    for (CachingReaderChunkForOwner* pChunk: m_chunks) {
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
    if (pChunk == nullptr) {
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
    if ((pChunk != nullptr) &&
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
            if (status.status != CHUNK_READ_SUCCESS) {
                // Discard chunks that are empty (EOF) or invalid
                freeChunk(pChunk);
            } else {
                // Insert or freshen the chunk in the MRU/LRU list after
                // obtaining ownership from the worker.
                freshenChunk(pChunk);
            }
        }
        if (status.status == TRACK_NOT_LOADED) {
            m_readerStatus = status.status;
        } else if (status.status == TRACK_LOADED) {
            m_readerStatus = status.status;
            // Reset the max. readable frame index
            m_readableFrameIndexRange = status.readableFrameIndexRange;
            // Free all chunks with sample data from a previous track
            freeAllChunks();
        }
        if (m_readerStatus == TRACK_LOADED) {
            // Adjust the readable frame index range after loading or reading
            m_readableFrameIndexRange = intersect(
                    m_readableFrameIndexRange,
                    status.readableFrameIndexRange);
        } else {
            // Reset the readable frame index range
            m_readableFrameIndexRange = mixxx::IndexRange();
        }
    }
}

SINT CachingReader::read(SINT startSample, SINT numSamples, bool reverse, CSAMPLE* buffer) {
    VERIFY_OR_DEBUG_ASSERT(buffer) {
        return 0;
    }

    // the samples are always read in forward direction
    // If reverse = true, the frames are copied in reverse order to the
    // destination buffer
    SINT sample = startSample;
    if (reverse) {
        // Start with the last sample in buffer
        sample -= numSamples;
    }

    // Check for bad inputs
    VERIFY_OR_DEBUG_ASSERT(sample % CachingReaderChunk::kChannels == 0) {
        // This problem is easy to fix, but this type of call should be
        // complained about loudly.
        --sample;
    }
    VERIFY_OR_DEBUG_ASSERT(numSamples % CachingReaderChunk::kChannels == 0) {
        --numSamples;
    }
    VERIFY_OR_DEBUG_ASSERT(numSamples >= 0) {
        QString temp = QString("Sample = %1").arg(sample);
        kLogger.debug() << "read() invalid arguments sample:" << sample
                 << "numSamples:" << numSamples << "buffer:" << buffer;
        return 0;
    }

    // If asked to read 0 samples, don't do anything. (this is a perfectly
    // reasonable request that happens sometimes. If no track is loaded, don't
    // do anything.
    if (numSamples == 0 || m_readerStatus != TRACK_LOADED) {
        return 0;
    }

    // Process messages from the reader thread.
    process();

    SINT samplesRead = 0;

    auto remainingFrameIndexRange =
            mixxx::IndexRange::forward(
                    CachingReaderChunk::samples2frames(sample),
                    CachingReaderChunk::samples2frames(numSamples));
    DEBUG_ASSERT(!remainingFrameIndexRange.empty());
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
            if (reverse) {
                SampleUtil::clear(&buffer[numSamples - samplesRead - prerollSamples], prerollSamples);
            } else {
                SampleUtil::clear(buffer, prerollSamples);
                buffer += prerollSamples;
            }
            samplesRead += prerollSamples;
            remainingFrameIndexRange.dropFrontRange(prerollFrames);
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

            const SINT firstCachingReaderChunkIndex =
                    CachingReaderChunk::indexForFrame(remainingFrameIndexRange.start());
            SINT lastCachingReaderChunkIndex =
                    CachingReaderChunk::indexForFrame(remainingFrameIndexRange.end() - 1);
            for (SINT chunkIndex = firstCachingReaderChunkIndex;
                    chunkIndex <= lastCachingReaderChunkIndex;
                    ++chunkIndex) {

                const CachingReaderChunkForOwner* const pChunk = lookupChunkAndFreshen(chunkIndex);
                // If the chunk is not in cache, then we must return an error.
                if (!pChunk || (pChunk->getState() != CachingReaderChunkForOwner::READY)) {
                    Counter("CachingReader::read(): Failed to read chunk on cache miss")++;
                    kLogger.warning()
                            << "Failed to fetch chunk with index"
                            << chunkIndex;
                    // Exit the loop and fill the remaining buffer with silence
                    break;
                }

                // Please note that m_readableFrameIndexRange might change with
                // every read operation! On a cache miss audio data will be
                // read from the audio source in lookupChunkAndFreshen() and
                // the readable frame index range might get adjusted if decoding
                // errors occur.
                remainingFrameIndexRange =
                        intersect(
                                remainingFrameIndexRange,
                                m_readableFrameIndexRange);
                if (remainingFrameIndexRange.empty()) {
                    // No more readable data available. Exit the loop and
                    // fill the remaining buffer with silence.
                    break;
                }

                mixxx::IndexRange bufferedFrameIndexRange;
                if (reverse) {
                    bufferedFrameIndexRange =
                            pChunk->readBufferedSampleFramesReverse(
                                    &buffer[numSamples - samplesRead],
                                    remainingFrameIndexRange);
                } else {
                    bufferedFrameIndexRange =
                            pChunk->readBufferedSampleFrames(
                                    buffer,
                                    remainingFrameIndexRange);
                }
                if (bufferedFrameIndexRange.empty()) {
                    // No more readable data available. Exit the loop and
                    // fill the remaining buffer with silence.
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
                    if (reverse) {
                        SampleUtil::clear(&buffer[numSamples - samplesRead - paddingSamples], paddingSamples);
                    } else {
                        SampleUtil::clear(buffer, paddingSamples);
                        buffer += paddingSamples;
                    }
                    samplesRead += paddingSamples;
                    remainingFrameIndexRange.dropFrontRange(paddingFrameIndexRange.length());
                }
                const SINT chunkSamples =
                        CachingReaderChunk::frames2samples(bufferedFrameIndexRange.length());
                if (!reverse) {
                    buffer += chunkSamples;
                }
                samplesRead += chunkSamples;
                remainingFrameIndexRange.dropFrontRange(bufferedFrameIndexRange.length());
            }
        }
    }
    // Finally fill the remaining buffer with silence.
    DEBUG_ASSERT(numSamples >= samplesRead);
    SampleUtil::clear(buffer, numSamples - samplesRead);
    return numSamples;
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

        const int firstCachingReaderChunkIndex = CachingReaderChunk::indexForFrame(readableFrameIndexRange.start());
        const int lastCachingReaderChunkIndex = CachingReaderChunk::indexForFrame(readableFrameIndexRange.end() - 1);
        for (int chunkIndex = firstCachingReaderChunkIndex; chunkIndex <= lastCachingReaderChunkIndex; ++chunkIndex) {
            CachingReaderChunkForOwner* pChunk = lookupChunk(chunkIndex);
            if (pChunk == nullptr) {
                shouldWake = true;
                pChunk = allocateChunkExpireLRU(chunkIndex);
                if (pChunk == nullptr) {
                    kLogger.debug() << "ERROR: Couldn't allocate spare CachingReaderChunk to make CachingReaderChunkReadRequest.";
                    continue;
                }
                // Do not insert the allocated chunk into the MRU/LRU list,
                // because it will be handed over to the worker immediately
                CachingReaderChunkReadRequest request(pChunk);
                pChunk->giveToWorker();
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
