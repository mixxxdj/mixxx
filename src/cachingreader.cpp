#include <QtDebug>
#include <QFileInfo>

#include "controlobject.h"
#include "controlobjectthread.h"

#include "cachingreader.h"
#include "trackinfoobject.h"
#include "sampleutil.h"
#include "util/counter.h"
#include "util/math.h"
#include "util/assert.h"

namespace {

// To prevent every bit of code having to guess how many samples
// forward it makes sense to keep in memory, the hinter can provide
// either 0 for a forward hint or -1 for a backward hint. We should
// be calculating an appropriate number of samples to go backward as
// some function of the latency, but for now just leave this as a
// constant. 2048 is a pretty good number of samples because 25ms
// latency corresponds to 1102.5 mono samples and we need double
// that for stereo samples.
const SINT kDefaultHintSamples = 1024 * CachingReaderWorker::kChunkChannels;

} // anonymous namespace

// currently CachingReaderWorker::kChunkLength is 65536 (0x10000);
// For 80 chunks we need 5242880 (0x500000) bytes (5 MiB) of Memory
//static
const int CachingReader::maximumChunksInMemory = 80;

CachingReader::CachingReader(QString group,
                             ConfigObject<ConfigValue>* config)
        : m_pConfig(config),
          m_chunkReadRequestFIFO(1024),
          m_readerStatusFIFO(1024),
          m_readerStatus(INVALID),
          m_mruChunk(NULL),
          m_lruChunk(NULL),
          m_sampleBuffer(CachingReaderWorker::kSamplesPerChunk * maximumChunksInMemory),
          m_maxReadableFrameIndex(Mixxx::AudioSource::getMinFrameIndex()),
          m_worker(group, &m_chunkReadRequestFIFO, &m_readerStatusFIFO) {

    m_allocatedChunks.reserve(m_sampleBuffer.size());

    CSAMPLE* bufferStart = m_sampleBuffer.data();

    // Divide up the allocated raw memory buffer into total_chunks
    // chunks. Initialize each chunk to hold nothing and add it to the free
    // list.
    for (int i = 0; i < maximumChunksInMemory; ++i) {
        Chunk* c = new Chunk;
        c->chunk_number = -1;
        c->frameCountRead = 0;
        c->frameCountTotal = 0;
        c->stereoSamples = bufferStart;
        c->next_lru = NULL;
        c->prev_lru = NULL;
        c->state = Chunk::FREE;

        m_chunks.push_back(c);
        m_freeChunks.push_back(c);

        bufferStart += CachingReaderWorker::kSamplesPerChunk;
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
    m_freeChunks.clear();
    m_allocatedChunks.clear();
    m_lruChunk = m_mruChunk = NULL;
    qDeleteAll(m_chunks);
}

// static
Chunk* CachingReader::removeFromLRUList(Chunk* chunk, Chunk* head) {
    if (chunk == NULL) {
        qDebug() << "ERROR: NULL chunk argument to removeFromLRUList";
        return NULL;
    }

    // Remove chunk from the doubly-linked list.
    Chunk* next = chunk->next_lru;
    Chunk* prev = chunk->prev_lru;

    if (next) {
        next->prev_lru = prev;
    }

    if (prev) {
        prev->next_lru = next;
    }

    chunk->next_lru = NULL;
    chunk->prev_lru = NULL;

    if (chunk == head)
        return next;

    return head;
}

// static
Chunk* CachingReader::insertIntoLRUList(Chunk* chunk, Chunk* head) {
    if (chunk == NULL) {
        qDebug() << "ERROR: NULL chunk argument to insertIntoLRUList";
        return NULL;
    }

    // Chunk is the new head of the list, so connect the head as the next from
    // chunk.
    chunk->next_lru = head;
    chunk->prev_lru = NULL;

    // If there are any elements in the list, point their prev pointer back at
    // chunk since it is the new head
    if (head) {
        head->prev_lru = chunk;
    }

    // Chunk is the new head
    return chunk;
}


void CachingReader::freeChunk(Chunk* pChunk) {
    int removed = m_allocatedChunks.remove(pChunk->chunk_number);

    // We'll tolerate not being in allocatedChunks because sometime you free a
    // chunk right after you allocated it.
    if (removed > 1) {
        qDebug() << "ERROR: freeChunk free'd a chunk that was multiply-allocated.";
    }

    // If this is the LRU chunk then set its previous LRU chunk to the LRU
    if (m_lruChunk == pChunk) {
        m_lruChunk = pChunk->prev_lru;
    }

    m_mruChunk = removeFromLRUList(pChunk, m_mruChunk);
    pChunk->state = Chunk::FREE;
    pChunk->chunk_number = -1;
    pChunk->frameCountRead = 0;
    pChunk->frameCountTotal = 0;
    m_freeChunks.push_back(pChunk);
}

void CachingReader::freeAllChunks() {
    m_allocatedChunks.clear();
    m_mruChunk = NULL;

    for (QVector<Chunk*>::const_iterator it = m_chunks.constBegin();
         it != m_chunks.constEnd(); ++it) {
        Chunk* pChunk = *it;

        // We will receive CHUNK_READ_INVALID for all pending chunk reads which
        // should free the chunks individually.
        if (pChunk->state == Chunk::READ_IN_PROGRESS) {
            continue;
        }

        if (pChunk->state != Chunk::FREE) {
            pChunk->state = Chunk::FREE;
            pChunk->chunk_number = -1;
            pChunk->frameCountRead = 0;
            pChunk->frameCountTotal = 0;
            pChunk->next_lru = NULL;
            pChunk->prev_lru = NULL;
            m_freeChunks.append(pChunk);
        }
    }
}

Chunk* CachingReader::allocateChunk(int chunk) {
    if (m_freeChunks.isEmpty()) {
        return NULL;
    }
    Chunk* pChunk = m_freeChunks.takeFirst();
    pChunk->state = Chunk::ALLOCATED;
    pChunk->chunk_number = chunk;

    //qDebug() << "Allocating chunk" << pChunk << pChunk->chunk_number;
    m_allocatedChunks.insert(pChunk->chunk_number, pChunk);

    // Insert the chunk into the least-recently-used linked list as the "most
    // recently used" item.
    m_mruChunk = insertIntoLRUList(pChunk, m_mruChunk);

    // If this chunk has no next least-recently-used pointer then it is the
    // least recently used chunk despite having just been allocated. This only
    // happens if this is the first allocated chunk.
    if (pChunk->next_lru == NULL) {
        m_lruChunk = pChunk;
    }
    return pChunk;
}

Chunk* CachingReader::allocateChunkExpireLRU(int chunk) {
    Chunk* pChunk = allocateChunk(chunk);
    if (pChunk == NULL) {
        if (m_lruChunk == NULL) {
            qDebug() << "ERROR: No LRU chunk to free in allocateChunkExpireLRU.";
            return NULL;
        }
        //qDebug() << "Expiring LRU" << m_lruChunk << m_lruChunk->chunk_number;
        freeChunk(m_lruChunk);
        pChunk = allocateChunk(chunk);
    }
    //qDebug() << "allocateChunkExpireLRU" << chunk << pChunk;
    return pChunk;
}

Chunk* CachingReader::lookupChunk(int chunk_number) {
    // Defaults to NULL if it's not in the hash.
    Chunk* chunk = m_allocatedChunks.value(chunk_number, NULL);

    // Make sure the allocated number matches the indexed chunk number.
    DEBUG_ASSERT(chunk == NULL || chunk_number == chunk->chunk_number);

    return chunk;
}

void CachingReader::freshenChunk(Chunk* pChunk) {
    // If this is the LRU chunk then set its previous LRU to be the new LRU.
    if (pChunk == m_lruChunk && pChunk->prev_lru != NULL) {
        m_lruChunk = pChunk->prev_lru;
    }
    // Remove the chunk from the LRU list and insert it at the head so that it
    // is now the most recently used chunk.
    m_mruChunk = removeFromLRUList(pChunk, m_mruChunk);
    m_mruChunk = insertIntoLRUList(pChunk, m_mruChunk);
}

Chunk* CachingReader::lookupChunkAndFreshen(int chunk_number) {
    Chunk* pChunk = lookupChunk(chunk_number);
    if (pChunk != NULL) {
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
        // qDebug() << "Got ReaderStatusUpdate:" << status.status
        //          << (status.chunk ? status.chunk->chunk_number : -1);
        if (status.status == TRACK_NOT_LOADED) {
            m_readerStatus = status.status;
            m_maxReadableFrameIndex = Mixxx::AudioSource::getMinFrameIndex();
        } else if (status.status == TRACK_LOADED) {
            freeAllChunks();
            m_readerStatus = status.status;
            m_maxReadableFrameIndex = status.maxReadableFrameIndex;
        } else {
            m_maxReadableFrameIndex = math_min(status.maxReadableFrameIndex, m_maxReadableFrameIndex);
            if ((status.status == CHUNK_READ_SUCCESS) ||
                    (status.status == CHUNK_READ_PARTIAL)) {
                Chunk* pChunk = status.chunk;
                // This should not be possible unless there is a bug in
                // CachingReaderWorker. If it is NULL then the only thing we can do
                // is to skip this ReaderStatusUpdate.
                DEBUG_ASSERT_AND_HANDLE(pChunk != NULL) {
                    continue;
                }
                // After a read success the state ought to be READ_IN_PROGRESS.
                DEBUG_ASSERT(pChunk->state == Chunk::READ_IN_PROGRESS);
                // Switch state to READ.
                pChunk->state = Chunk::READ;
            } else if (status.status == CHUNK_READ_EOF) {
                Chunk* pChunk = status.chunk;
                if (pChunk == NULL) {
                    qDebug() << "ERROR: status.chunk is NULL in CHUNK_READ_EOF ReaderStatusUpdate. Ignoring update.";
                    continue;
                }
                DEBUG_ASSERT(pChunk->state == Chunk::READ_IN_PROGRESS);
                freeChunk(pChunk);
            } else if (status.status == CHUNK_READ_INVALID) {
                qDebug() << "WARNING: READER THREAD RECEIVED INVALID CHUNK READ";
                Chunk* pChunk = status.chunk;
                if (pChunk == NULL) {
                    qDebug() << "ERROR: status.chunk is NULL in CHUNK_READ_INVALID ReaderStatusUpdate. Ignoring update.";
                    continue;
                }
                DEBUG_ASSERT(pChunk->state == Chunk::READ_IN_PROGRESS);
                freeChunk(pChunk);
            }
        }
    }
}

int CachingReader::read(int sample, int numSamples, CSAMPLE* buffer) {
    // Check for bad inputs
    DEBUG_ASSERT_AND_HANDLE(sample % CachingReaderWorker::kChunkChannels == 0) {
        // This problem is easy to fix, but this type of call should be
        // complained about loudly.
        --sample;
    }
    DEBUG_ASSERT_AND_HANDLE(numSamples % CachingReaderWorker::kChunkChannels == 0) {
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

    DEBUG_ASSERT(0 == (sample % CachingReaderWorker::kChunkChannels));
    SINT frameIndex = sample / CachingReaderWorker::kChunkChannels;
    DEBUG_ASSERT(0 == (numSamples % CachingReaderWorker::kChunkChannels));
    SINT numFrames = numSamples / CachingReaderWorker::kChunkChannels;

    // TODO: is it possible to move this code out of caching reader
    // and into enginebuffer?  It doesn't quite make sense here, although
    // it makes preroll completely transparent to the rest of the code
    // if we're in preroll...
    if (Mixxx::AudioSource::getMinFrameIndex() > frameIndex) {
        const SINT prerollFrames = Mixxx::AudioSource::getMinFrameIndex() - frameIndex;
        const SINT prerollSamples = prerollFrames * CachingReaderWorker::kChunkChannels;
        SampleUtil::clear(buffer, prerollSamples);
        buffer += prerollSamples;
        samplesRead += prerollSamples;
        frameIndex = Mixxx::AudioSource::getMinFrameIndex();
        numFrames -= prerollFrames;
    }

    const SINT maxReadableFrameIndex = math_min(frameIndex + numFrames, m_maxReadableFrameIndex);
    if (maxReadableFrameIndex > frameIndex) {
        const int firstChunkIndex = chunkForFrame(frameIndex);
        const int lastChunkIndex = chunkForFrame(maxReadableFrameIndex - 1);

        for (int chunkIndex = firstChunkIndex; chunkIndex <= lastChunkIndex; ++chunkIndex) {
            Chunk* current = lookupChunkAndFreshen(chunkIndex);

            // If the chunk is not in cache, then we must return an error.
            if (current == NULL || current->state != Chunk::READ) {
                // qDebug() << "Couldn't get chunk " << chunk_num
                //          << " in read() of [" << sample << "," << (sample + samples_remaining)
                //          << "] chunks " << firstChunkIndex << "-" << lastChunkIndex;

                // Something is wrong. Break out of the loop, that should fill the
                // samples requested with zeroes.
                Counter("CachingReader::read cache miss")++;
                break;
            }

            const SINT minChunkFrameIndex = CachingReaderWorker::frameForChunk(chunkIndex);
            DEBUG_ASSERT(minChunkFrameIndex <= frameIndex);
            const SINT maxChunkFrameIndex = math_min(minChunkFrameIndex + current->frameCountTotal, maxReadableFrameIndex);
            if (maxChunkFrameIndex <= frameIndex) {
                // no more samples to read
                break;
            }

            const SINT sampleOffset = (frameIndex - minChunkFrameIndex) * CachingReaderWorker::kChunkChannels;
            const SINT samplesToCopy = (maxChunkFrameIndex - frameIndex) * CachingReaderWorker::kChunkChannels;
            SampleUtil::copy(buffer, current->stereoSamples + sampleOffset, samplesToCopy);
            buffer += samplesToCopy;
            samplesRead += samplesToCopy;
            frameIndex = maxChunkFrameIndex;
        }
    }

    // If we didn't supply all the samples requested, that probably means we're
    // at the end of the file, or something is wrong. Provide zeroes and pretend
    // all is well. The caller can't be bothered to check how long the file is.
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

        DEBUG_ASSERT(0 == (hint.sample % CachingReaderWorker::kChunkChannels));
        const SINT hintFrame = hint.sample / CachingReaderWorker::kChunkChannels;
        DEBUG_ASSERT(0 == (hint.length % CachingReaderWorker::kChunkChannels));
        const SINT hintFrameCount = hint.length / CachingReaderWorker::kChunkChannels;

        SINT minReadableFrameIndex = hintFrame;
        SINT maxReadableFrameIndex = hintFrame + hintFrameCount;
        Mixxx::AudioSource::clampFrameInterval(&minReadableFrameIndex, &maxReadableFrameIndex, m_maxReadableFrameIndex);
        if (minReadableFrameIndex >= maxReadableFrameIndex) {
            // skip empty frame interval silently
            continue;
        }

        const int firstChunkIndex = chunkForFrame(minReadableFrameIndex);
        const int lastChunkIndex = chunkForFrame(maxReadableFrameIndex - 1);
        for (int chunkIndex = firstChunkIndex; chunkIndex <= lastChunkIndex; ++chunkIndex) {
            Chunk* pChunk = lookupChunk(chunkIndex);
            if (pChunk == NULL) {
                shouldWake = true;
                Chunk* pChunk = allocateChunkExpireLRU(chunkIndex);
                if (pChunk == NULL) {
                    qDebug() << "ERROR: Couldn't allocate spare Chunk to make ChunkReadRequest.";
                    continue;
                }
                pChunk->state = Chunk::READ_IN_PROGRESS;
                ChunkReadRequest request;
                request.chunk = pChunk;
                // qDebug() << "Requesting read of chunk" << current << "into" << pChunk;
                // qDebug() << "Requesting read into " << request.chunk->data;
                if (m_chunkReadRequestFIFO.write(&request, 1) != 1) {
                    qDebug() << "ERROR: Could not submit read request for "
                             << chunkIndex;
                }
                //qDebug() << "Checking chunk " << current << " shouldWake:" << shouldWake << " chunksToRead" << m_chunksToRead.size();
            } else if (pChunk->state == Chunk::READ) {
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
