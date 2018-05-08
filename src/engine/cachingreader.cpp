#include <QtDebug>
#include <QFileInfo>

#include "engine/cachingreader.h"
#include "control/controlobject.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/counter.h"
#include "util/math.h"
#include "util/sample.h"

namespace {

// NOTE(uklotzde): The following comment has been adopted without
// modifications and should be rephrased.
//
// To prevent every bit of code having to guess how many samples
// forward it makes sense to keep in memory, the hinter can provide
// either 0 for a forward hint or -1 for a backward hint. We should
// be calculating an appropriate number of samples to go backward as
// some function of the latency, but for now just leave this as a
// constant. 2048 is a pretty good number of samples because 25ms
// latency corresponds to 1102.5 mono samples and we need double
// that for stereo samples.
const SINT kDefaultHintSamples = 1024 * CachingReaderChunk::kChannels;

} // anonymous namespace

// currently CachingReaderWorker::kCachingReaderChunkLength is 65536 (0x10000);
// For 80 chunks we need 5242880 (0x500000) bytes (5 MiB) of Memory
//static
const int CachingReader::maximumCachingReaderChunksInMemory = 80;

CachingReader::CachingReader(QString group,
                             UserSettingsPointer config)
        : m_pConfig(config),
          m_chunkReadRequestFIFO(1024),
          m_readerStatusFIFO(1024),
          m_readerStatus(INVALID),
          m_mruCachingReaderChunk(nullptr),
          m_lruCachingReaderChunk(nullptr),
          m_sampleBuffer(CachingReaderChunk::kSamples * maximumCachingReaderChunksInMemory),
          m_maxReadableFrameIndex(mixxx::AudioSource::getMinFrameIndex()),
          m_worker(group, &m_chunkReadRequestFIFO, &m_readerStatusFIFO) {

    m_allocatedCachingReaderChunks.reserve(maximumCachingReaderChunksInMemory);

    CSAMPLE* bufferStart = m_sampleBuffer.data();

    // Divide up the allocated raw memory buffer into total_chunks
    // chunks. Initialize each chunk to hold nothing and add it to the free
    // list.
    for (int i = 0; i < maximumCachingReaderChunksInMemory; ++i) {
        CachingReaderChunkForOwner* c = new CachingReaderChunkForOwner(bufferStart);

        m_chunks.push_back(c);
        m_freeChunks.push_back(c);

        bufferStart += CachingReaderChunk::kSamples;
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

    //qDebug() << "Allocating chunk" << pChunk << pChunk->getIndex();
    m_allocatedCachingReaderChunks.insert(chunkIndex, pChunk);

    return pChunk;
}

CachingReaderChunkForOwner* CachingReader::allocateChunkExpireLRU(SINT chunkIndex) {
    CachingReaderChunkForOwner* pChunk = allocateChunk(chunkIndex);
    if (pChunk == nullptr) {
        if (m_lruCachingReaderChunk == nullptr) {
            qWarning() << "ERROR: No LRU chunk to free in allocateChunkExpireLRU.";
            return nullptr;
        }
        freeChunk(m_lruCachingReaderChunk);
        pChunk = allocateChunk(chunkIndex);
    }
    //qDebug() << "allocateChunkExpireLRU" << chunk << pChunk;
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
            m_maxReadableFrameIndex = status.maxReadableFrameIndex;
            // Free all chunks with sample data from a previous track
            freeAllChunks();
        }
        // Adjust the max. readable frame index
        if (m_readerStatus == TRACK_LOADED) {
            m_maxReadableFrameIndex = math_min(status.maxReadableFrameIndex, m_maxReadableFrameIndex);
        } else {
            m_maxReadableFrameIndex = mixxx::AudioSource::getMinFrameIndex();
        }
    }
}

SINT CachingReader::read(SINT startSample, SINT numSamples, bool reverse, CSAMPLE* buffer) {
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
    if (numSamples < 0 || !buffer) {
        QString temp = QString("Sample = %1").arg(sample);
        qDebug() << "CachingReader::read() invalid arguments sample:" << sample
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

    SINT frameIndex = CachingReaderChunk::samples2frames(sample);
    SINT numFrames = CachingReaderChunk::samples2frames(numSamples);

    // Fill the buffer up to the first readable sample with
    // silence. This may happen when the engine is in preroll,
    // i.e. if the frame index points a region before the first
    // track sample.
    if (mixxx::AudioSource::getMinFrameIndex() > frameIndex) {
        const SINT prerollFrames = math_min(numFrames,
                mixxx::AudioSource::getMinFrameIndex() - frameIndex);
        const SINT prerollSamples = CachingReaderChunk::frames2samples(prerollFrames);
        if (reverse) {
            SampleUtil::clear(&buffer[numSamples - prerollSamples], prerollSamples);
        } else {
            SampleUtil::clear(buffer, prerollSamples);
            buffer += prerollSamples;
        }

        samplesRead += prerollSamples;
        frameIndex += prerollFrames;
        numFrames -= prerollFrames;
    }

    // Read the actual samples from the audio source into the
    // buffer. The buffer will be filled with silence for every
    // unreadable sample or samples outside of the track region
    // later at the end of this function.
    if (numSamples > samplesRead) {
        // If any unread samples from the track are left the current
        // frame index must be at or beyond the first track sample.
        DEBUG_ASSERT(mixxx::AudioSource::getMinFrameIndex() <= frameIndex);

        SINT maxReadableFrameIndex = math_min(frameIndex + numFrames, m_maxReadableFrameIndex);
        if (maxReadableFrameIndex > frameIndex) {
            // The intersection between the readable samples from the track
            // and the requested samples is not empty, so start reading.

            const SINT firstCachingReaderChunkIndex = CachingReaderChunk::indexForFrame(frameIndex);
            SINT lastCachingReaderChunkIndex = CachingReaderChunk::indexForFrame(maxReadableFrameIndex - 1);
            for (SINT chunkIndex = firstCachingReaderChunkIndex; chunkIndex <= lastCachingReaderChunkIndex; ++chunkIndex) {

                const CachingReaderChunkForOwner* const pChunk = lookupChunkAndFreshen(chunkIndex);
                // If the chunk is not in cache, then we must return an error.
                if (!pChunk || (pChunk->getState() != CachingReaderChunkForOwner::READY)) {
                    Counter("CachingReader::read(): Failed to read chunk on cache miss")++;
                    // Exit the loop and fill the remaining buffer with silence
                    break;
                }

                // Please note that m_maxReadableFrameIndex might change with
                // every read operation! On a cache miss audio data will be
                // read from the audio source in lookupChunkAndFreshen() and
                // the max. readable frame index might be adjusted if decoding
                // errors occur.
                maxReadableFrameIndex = math_min(maxReadableFrameIndex, m_maxReadableFrameIndex);
                if (maxReadableFrameIndex <= frameIndex) {
                    // No more readable data available. Exit the loop and
                    // fill the remaining buffer with silence.
                    break;
                }
                DEBUG_ASSERT(0 < maxReadableFrameIndex);
                lastCachingReaderChunkIndex = CachingReaderChunk::indexForFrame(maxReadableFrameIndex - 1);

                const SINT chunkFrameIndex = CachingReaderChunk::frameForIndex(chunkIndex);
                DEBUG_ASSERT(chunkFrameIndex <= frameIndex);
                DEBUG_ASSERT((chunkIndex == firstCachingReaderChunkIndex) ||
                        (chunkFrameIndex == frameIndex));
                const SINT chunkFrameOffset = frameIndex - chunkFrameIndex;
                DEBUG_ASSERT(chunkFrameOffset >= 0);
                const SINT chunkFrameCount = math_min(
                        pChunk->getFrameCount(),
                        maxReadableFrameIndex - chunkFrameIndex);
                if (chunkFrameCount < chunkFrameOffset) {
                    // No more readable data available from this chunk (and
                    // consequently all following chunks). Exit the loop and
                    // fill the remaining buffer with silence.
                    break;
                }

                const SINT framesToCopy = chunkFrameCount - chunkFrameOffset;
                DEBUG_ASSERT(framesToCopy >= 0);
                const SINT chunkSampleOffset = CachingReaderChunk::frames2samples(chunkFrameOffset);
                const SINT samplesToCopy = CachingReaderChunk::frames2samples(framesToCopy);

                if (reverse) {
                    pChunk->copySamplesReverse(&buffer[numSamples - samplesRead - samplesToCopy], chunkSampleOffset, samplesToCopy);
                } else {
                    pChunk->copySamples(buffer, chunkSampleOffset, samplesToCopy);
                    buffer += samplesToCopy;
                }
                samplesRead += samplesToCopy;
                frameIndex += framesToCopy;
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

    for (HintVector::const_iterator it = hintList.constBegin();
         it != hintList.constEnd(); ++it) {
        // Copy, don't use reference.
        Hint hint = *it;

        // Handle some special length values
        if (hint.length == 0) {
            hint.length = kDefaultHintSamples;
        } else if (hint.length == -1) {
            hint.sample -= kDefaultHintSamples;
            hint.length = kDefaultHintSamples;
            if (hint.sample < 0) {
                hint.length += hint.sample;
                hint.sample = 0;
            }
        }
        if (hint.length < 0) {
            qDebug() << "ERROR: Negative hint length. Ignoring.";
            continue;
        }

        const SINT hintFrame = CachingReaderChunk::samples2frames(hint.sample);
        const SINT hintFrameCount = CachingReaderChunk::samples2frames(hint.length);

        SINT minReadableFrameIndex = hintFrame;
        SINT maxReadableFrameIndex = hintFrame + hintFrameCount;
        mixxx::AudioSource::clampFrameInterval(&minReadableFrameIndex, &maxReadableFrameIndex, m_maxReadableFrameIndex);
        if (minReadableFrameIndex >= maxReadableFrameIndex) {
            // skip empty frame interval silently
            continue;
        }

        const int firstCachingReaderChunkIndex = CachingReaderChunk::indexForFrame(minReadableFrameIndex);
        const int lastCachingReaderChunkIndex = CachingReaderChunk::indexForFrame(maxReadableFrameIndex - 1);
        for (int chunkIndex = firstCachingReaderChunkIndex; chunkIndex <= lastCachingReaderChunkIndex; ++chunkIndex) {
            CachingReaderChunkForOwner* pChunk = lookupChunk(chunkIndex);
            if (pChunk == nullptr) {
                shouldWake = true;
                pChunk = allocateChunkExpireLRU(chunkIndex);
                if (pChunk == nullptr) {
                    qDebug() << "ERROR: Couldn't allocate spare CachingReaderChunk to make CachingReaderChunkReadRequest.";
                    continue;
                }
                // Do not insert the allocated chunk into the MRU/LRU list,
                // because it will be handed over to the worker immediately
                CachingReaderChunkReadRequest request(pChunk);
                pChunk->giveToWorker();
                // qDebug() << "Requesting read of chunk" << current << "into" << pChunk;
                // qDebug() << "Requesting read into " << request.chunk->data;
                if (m_chunkReadRequestFIFO.write(&request, 1) != 1) {
                    qWarning() << "ERROR: Could not submit read request for "
                             << chunkIndex;
                    // Revoke the chunk from the worker and free it
                    pChunk->takeFromWorker();
                    freeChunk(pChunk);
                }
                //qDebug() << "Checking chunk " << current << " shouldWake:" << shouldWake << " chunksToRead" << m_chunksToRead.size();
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
