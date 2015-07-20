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
          m_iTrackNumFramesCallbackSafe(0) {

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

    m_pWorker = new CachingReaderWorker(group,
            &m_chunkReadRequestFIFO,
            &m_readerStatusFIFO);

    // Forward signals from worker
    connect(m_pWorker, SIGNAL(trackLoading()),
            this, SIGNAL(trackLoading()),
            Qt::DirectConnection);
    connect(m_pWorker, SIGNAL(trackLoaded(TrackPointer, int, int)),
            this, SIGNAL(trackLoaded(TrackPointer, int, int)),
            Qt::DirectConnection);
    connect(m_pWorker, SIGNAL(trackLoadFailed(TrackPointer, QString)),
            this, SIGNAL(trackLoadFailed(TrackPointer, QString)),
            Qt::DirectConnection);

    m_pWorker->start(QThread::HighPriority);
}


CachingReader::~CachingReader() {

    m_pWorker->quitWait();
    delete m_pWorker;
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
    m_pWorker->newTrack(pTrack);
    m_pWorker->workReady();
}

void CachingReader::process() {
    ReaderStatusUpdate status;
    while (m_readerStatusFIFO.read(&status, 1) == 1) {
        // qDebug() << "Got ReaderStatusUpdate:" << status.status
        //          << (status.chunk ? status.chunk->chunk_number : -1);
        if (status.status == TRACK_NOT_LOADED) {
            m_readerStatus = status.status;
        } else if (status.status == TRACK_LOADED) {
            freeAllChunks();
            m_readerStatus = status.status;
            m_iTrackNumFramesCallbackSafe = status.trackFrameCount;
        } else if ((status.status == CHUNK_READ_SUCCESS) ||
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

int CachingReader::read(int sample, int num_samples, CSAMPLE* buffer) {
    // Check for bad inputs
    DEBUG_ASSERT_AND_HANDLE(sample % 2 == 0) {
        // This problem is easy to fix, but this type of call should be
        // complained about loudly.
        --sample;
    }
    DEBUG_ASSERT_AND_HANDLE(num_samples % 2 == 0) {
        --num_samples;
    }
    if (num_samples < 0 || !buffer) {
        QString temp = QString("Sample = %1").arg(sample);
        qDebug() << "CachingReader::read() invalid arguments sample:" << sample
                 << "num_samples:" << num_samples << "buffer:" << buffer;
        return 0;
    }

    // If asked to read 0 samples, don't do anything. (this is a perfectly
    // reasonable request that happens sometimes. If no track is loaded, don't
    // do anything.
    if (num_samples == 0 || m_readerStatus != TRACK_LOADED) {
        return 0;
    }

    // Process messages from the reader thread.
    process();

    int samples_remaining = num_samples;


    // TODO: is it possible to move this code out of caching reader
    // and into enginebuffer?  It doesn't quite make sense here, although
    // it makes preroll completely transparent to the rest of the code
    // if we're in preroll...
    if (sample < 0) {
        int zero_samples = math_min(-sample, samples_remaining);
        DEBUG_ASSERT(0 <= zero_samples);
        DEBUG_ASSERT(zero_samples <= samples_remaining);
        SampleUtil::clear(buffer, zero_samples);
        samples_remaining -= zero_samples;
        if (samples_remaining == 0) {
            //everything is zeros, easy
            return zero_samples;
        }
        buffer += zero_samples;
        sample += zero_samples;
        DEBUG_ASSERT(0 <= sample);
    }

    DEBUG_ASSERT(0 == (sample % CachingReaderWorker::kChunkChannels));
    const int frame = sample / CachingReaderWorker::kChunkChannels;
    DEBUG_ASSERT(0 == (samples_remaining % CachingReaderWorker::kChunkChannels));
    int frames_remaining = samples_remaining / CachingReaderWorker::kChunkChannels;
    const int start_frame = math_min(m_iTrackNumFramesCallbackSafe, frame);
    const int start_chunk = chunkForFrame(start_frame);
    const int end_frame = math_min(m_iTrackNumFramesCallbackSafe, frame + (frames_remaining - 1));
    const int end_chunk = chunkForFrame(end_frame);

    int current_frame = frame;

    // Sanity checks
    if (start_chunk > end_chunk) {
        qDebug() << "CachingReader::read() bad chunk range to read ["
                 << start_chunk << end_chunk << "]";
        return 0;
    }

    for (int chunk_num = start_chunk; chunk_num <= end_chunk; chunk_num++) {
        Chunk* current = lookupChunkAndFreshen(chunk_num);

        // If the chunk is not in cache, then we must return an error.
        if (current == NULL || current->state != Chunk::READ) {
            // qDebug() << "Couldn't get chunk " << chunk_num
            //          << " in read() of [" << sample << "," << (sample + samples_remaining)
            //          << "] chunks " << start_chunk << "-" << end_chunk;

            // Something is wrong. Break out of the loop, that should fill the
            // samples requested with zeroes.
            Counter("CachingReader::read cache miss")++;
            break;
        }

        int chunk_start_frame = CachingReaderWorker::frameForChunk(chunk_num);
        const int chunk_frame_offset = current_frame - chunk_start_frame;
        int chunk_remaining_frames = current->frameCountTotal - chunk_frame_offset;

        // More sanity checks
        if (current_frame < chunk_start_frame) {
            qDebug() << "CachingReader::read() bad chunk parameters"
                     << "chunk_start_frame" << chunk_start_frame
                     << "current_frame" << current_frame;
            break;
        }

        // If we're past the start_chunk then current_sample should be
        // chunk_start_sample.
        if (start_chunk != chunk_num && chunk_start_frame != current_frame) {
            qDebug() << "CachingReader::read() bad chunk parameters"
                     << "chunk_num" << chunk_num
                     << "start_chunk" << start_chunk
                     << "chunk_start_sample" << chunk_start_frame
                     << "current_sample" << current_frame;
            break;
        }

        if (frames_remaining < 0) {
            qDebug() << "CachingReader::read() bad samples remaining"
                     << frames_remaining;
            break;
        }

        // It is completely possible that chunk_remaining_samples is less than
        // zero. If the caller is trying to read from beyond the end of the
        // file, then this can happen. We should tolerate it.
        if (chunk_remaining_frames < 0) {
            chunk_remaining_frames = 0;
        }
        const int frames_to_read = math_clamp(frames_remaining, 0, chunk_remaining_frames);

        // If we did not decide to read any samples from this chunk then that
        // means we have exhausted all the samples in the song.
        if (frames_to_read == 0) {
            break;
        }

        // samples_to_read should be non-negative and even
        if (frames_to_read < 0) {
            qDebug() << "CachingReader::read() frames_to_read invalid"
                     << frames_to_read;
            break;
        }

        const int chunk_sample_offset = chunk_frame_offset * CachingReaderWorker::kChunkChannels;
        const int samples_to_read = frames_to_read * CachingReaderWorker::kChunkChannels;
        SampleUtil::copy(buffer, current->stereoSamples + chunk_sample_offset, samples_to_read);
        buffer += samples_to_read;
        samples_remaining -= samples_to_read;
        current_frame += frames_to_read;
        frames_remaining -= frames_to_read;
    }

    // If we didn't supply all the samples requested, that probably means we're
    // at the end of the file, or something is wrong. Provide zeroes and pretend
    // all is well. The caller can't be bothered to check how long the file is.
    SampleUtil::clear(buffer, samples_remaining);
    buffer += samples_remaining;
    samples_remaining -= samples_remaining;
    current_frame += frames_remaining;
    frames_remaining -= frames_remaining;

    return num_samples - samples_remaining;
}

void CachingReader::hintAndMaybeWake(const HintVector& hintList) {
    // If no file is loaded, skip.
    if (m_readerStatus != TRACK_LOADED) {
        return;
    }

    // To prevent every bit of code having to guess how many samples
    // forward it makes sense to keep in memory, the hinter can provide
    // either 0 for a forward hint or -1 for a backward hint. We should
    // be calculating an appropriate number of samples to go backward as
    // some function of the latency, but for now just leave this as a
    // constant. 2048 is a pretty good number of samples because 25ms
    // latency corresponds to 1102.5 mono samples and we need double
    // that for stereo samples.
    const int default_samples = 1024 * CachingReaderWorker::kChunkChannels;

    // For every chunk that the hints indicated, check if it is in the cache. If
    // any are not, then wake.
    bool shouldWake = false;

    for (HintVector::const_iterator it = hintList.constBegin();
         it != hintList.constEnd(); ++it) {
        // Copy, don't use reference.
        Hint hint = *it;

        if (hint.length == 0) {
            hint.length = default_samples;
        } else if (hint.length == -1) {
            hint.sample -= default_samples;
            hint.length = default_samples;
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
        const int frame = hint.sample / CachingReaderWorker::kChunkChannels;
        DEBUG_ASSERT(0 == (hint.length % CachingReaderWorker::kChunkChannels));
        const int frame_count = hint.length / CachingReaderWorker::kChunkChannels;
        const int start_frame = math_clamp(frame, 0,
                                      m_iTrackNumFramesCallbackSafe);
        const int start_chunk = chunkForFrame(start_frame);
        int end_frame = math_clamp(frame + (frame_count - 1), 0,
                m_iTrackNumFramesCallbackSafe);
        const int end_chunk = chunkForFrame(end_frame);

        for (int current = start_chunk; current <= end_chunk; ++current) {
            Chunk* pChunk = lookupChunk(current);
            if (pChunk == NULL) {
                shouldWake = true;
                Chunk* pChunk = allocateChunkExpireLRU(current);
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
                             << current;
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
        m_pWorker->workReady();
    }
}
