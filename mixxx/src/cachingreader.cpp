
#include <math.h>

#include <QtDebug>
#include <QFileInfo>

#include "controlobject.h"
#include "controlobjectthread.h"

#include "cachingreader.h"
#include "trackinfoobject.h"
#include "soundsourceproxy.h"
#include "sampleutil.h"


// There's a little math to this, but not much: 48khz stereo audio is 384kb/sec
// if using float samples. We want the chunk size to be a power of 2 so it's
// easier to memory align, and roughly 1/2 - 1/4th of a second of audio. 2**17
// and 2**16 are nice candidates. 2**16 is 170ms of audio, which is well above
// (hopefully) the latencies people are seeing. at 10ms latency, one chunk is
// enough for 17 callbacks. We may need to tweak this later.

// Must be divisible by 8, 4, and 2. Just pick a power of 2.
#define CHUNK_LENGTH 65536
//#define CHUNK_LENGTH 524288

const int CachingReader::kChunkLength = CHUNK_LENGTH;
const int CachingReader::kSamplesPerChunk = CHUNK_LENGTH / sizeof(CSAMPLE);

CachingReader::CachingReader(const char* _group,
                             ConfigObject<ConfigValue>* _config) :
        m_pGroup(_group),
        m_pConfig(_config),
        m_chunkReadRequestFIFO(1024),
        m_readerStatusFIFO(1024),
        m_readerStatus(INVALID),
        m_mruChunk(NULL),
        m_lruChunk(NULL),
        m_pRawMemoryBuffer(NULL),
        m_pCurrentSoundSource(NULL),
        m_iTrackSampleRate(0),
        m_iTrackNumSamples(0),
        m_iTrackNumSamplesCallbackSafe(0),
        m_pSample(NULL) {
    initialize();
}

CachingReader::~CachingReader() {
    m_freeChunks.clear();
    m_allocatedChunks.clear();
    m_lruChunk = m_mruChunk = NULL;

    for (int i=0; i < m_chunks.size(); i++) {
        Chunk* c = m_chunks[i];
        delete c;
    }

    delete [] m_pSample;

    delete [] m_pRawMemoryBuffer;
    m_pRawMemoryBuffer = NULL;

    delete m_pCurrentSoundSource;
}

void CachingReader::initialize() {
    int memory_to_use = 5000000; // 5mb, TODO
    Q_ASSERT(memory_to_use >= kChunkLength);

    // Only allocate as many bytes as we will actually use.
    memory_to_use -= (memory_to_use % kChunkLength);

    m_pSample = new SAMPLE[kSamplesPerChunk];

    int total_chunks = memory_to_use / kChunkLength;

    //qDebug() << "CachingReader using" << memory_to_use << "bytes.";

    int rawMemoryBufferLength = kSamplesPerChunk * total_chunks;
    m_pRawMemoryBuffer = new CSAMPLE[rawMemoryBufferLength];

    m_allocatedChunks.reserve(total_chunks);

    CSAMPLE* bufferStart = m_pRawMemoryBuffer;

    // Divide up the allocated raw memory buffer into total_chunks
    // chunks. Initialize each chunk to hold nothing and add it to the free
    // list.
    for (int i=0; i < total_chunks; i++) {
        Chunk* c = new Chunk;
        c->chunk_number = -1;
        c->length = 0;
        c->data = bufferStart;
        c->next_lru = NULL;
        c->prev_lru = NULL;

        m_chunks.push_back(c);
        m_freeChunks.push_back(c);

        bufferStart += kSamplesPerChunk;
    }
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

    pChunk->chunk_number = -1;
    pChunk->length = 0;
    m_freeChunks.push_back(pChunk);
}

void CachingReader::freeAllChunks() {
    m_allocatedChunks.clear();
    m_mruChunk = NULL;

    QSet<Chunk*> reserved = QSet<Chunk*>::fromList(m_chunksBeingRead.values());

    for (int i=0; i < m_chunks.size(); i++) {
        Chunk* c = m_chunks[i];
        if (reserved.contains(c)) {
            continue;
        }
        if (!m_freeChunks.contains(c)) {
            c->chunk_number = -1;
            c->length = 0;
            c->next_lru = NULL;
            c->prev_lru = NULL;
            m_freeChunks.push_back(c);
        }
    }
}

Chunk* CachingReader::allocateChunk() {
    if (m_freeChunks.count() == 0)
        return NULL;
    return m_freeChunks.takeFirst();
}

Chunk* CachingReader::allocateChunkExpireLRU() {
    Chunk* chunk = allocateChunk();
    if (chunk == NULL) {
        if (m_lruChunk == NULL) {
            qDebug() << "ERROR: No LRU chunk to free in allocateChunkExpireLRU.";
            return NULL;
        }
        //qDebug() << "Expiring LRU" << m_lruChunk << m_lruChunk->chunk_number;
        freeChunk(m_lruChunk);
        chunk = allocateChunk();
    }
    //qDebug() << "allocateChunkExpireLRU" << chunk;
    return chunk;
}

Chunk* CachingReader::lookupChunk(int chunk_number) {
    // Defaults to NULL if it's not in the hash.
    Chunk* chunk = NULL;

    if (m_allocatedChunks.contains(chunk_number)) {
        chunk = m_allocatedChunks.value(chunk_number);

        // Make sure we're all in agreement here.
        if (chunk_number != chunk->chunk_number) {
            qDebug() << "ERROR: Inconsistent chunk has chunk_number that doesn't match allocated-chunks key.";
        }

        // If this is the LRU chunk then set the previous LRU to the new LRU
        if (chunk == m_lruChunk && chunk->prev_lru != NULL) {
            m_lruChunk = chunk->prev_lru;
        }
        // Remove the chunk from the list and insert it at the head.
        m_mruChunk = removeFromLRUList(chunk, m_mruChunk);
        m_mruChunk = insertIntoLRUList(chunk, m_mruChunk);
    }

    return chunk;
}

void CachingReader::processChunkReadRequest(ChunkReadRequest* request,
                                            ReaderStatusUpdate* update) {
    int chunk_number = request->chunk->chunk_number;
    //qDebug() << "Processing ChunkReadRequest for" << chunk_number;
    update->status = CHUNK_READ_INVALID;
    update->chunk = request->chunk;
    update->chunk->length = 0;

    if (m_pCurrentSoundSource == NULL || chunk_number < 0) {
        return;
    }

    // Stereo samples
    int sample_position = sampleForChunk(chunk_number);
    int samples_remaining = m_iTrackNumSamples - sample_position;
    int samples_to_read = math_min(kSamplesPerChunk, samples_remaining);

    // Bogus chunk number
    if (samples_to_read <= 0) {
        update->status = CHUNK_READ_EOF;
        return;
    }

    m_pCurrentSoundSource->seek(sample_position);
    int samples_read = m_pCurrentSoundSource->read(samples_to_read,
                                                   m_pSample);

    // If we've run out of music, the SoundSource can return 0 samples.
    // Remember that SoundSourc->getLength() (which is m_iTrackNumSamples) can
    // lie to us about the length of the song!
    if (samples_read <= 0) {
        update->status = CHUNK_READ_EOF;
        return;
    }

    // TODO(XXX) This loop can't be done with a memcpy, but could be done with
    // SSE.
    CSAMPLE* buffer = request->chunk->data;
    //qDebug() << "Reading into " << buffer;
    SampleUtil::convert(buffer, m_pSample, samples_read);
    update->status = CHUNK_READ_SUCCESS;
    update->chunk->length = samples_read;
}

void CachingReader::newTrack(TrackPointer pTrack) {
    m_trackQueueMutex.lock();
    m_trackQueue.enqueue(pTrack);
    m_trackQueueMutex.unlock();
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
            m_iTrackNumSamplesCallbackSafe = status.trackNumSamples;
        } else if (status.status == CHUNK_READ_SUCCESS) {
            Chunk* pChunk = status.chunk;
            if (pChunk == NULL) {
                qDebug() << "ERROR: status.chunk is NULL in CHUNK_READ_SUCCESS ReaderStatusUpdate. Ignoring update.";
                continue;
            }
            Chunk* pChunk2 = m_chunksBeingRead.take(pChunk->chunk_number);
            if (pChunk2 != pChunk) {
                qDebug() << "Mismatch in requested chunk to read!";
            }

            Chunk* pAlreadyExisting = lookupChunk(pChunk->chunk_number);
            // If this chunk is already in the cache, then we just freshened
            // it. Free this chunk.
            if (pAlreadyExisting != NULL) {
                qDebug() << "CHUNK" << pChunk->chunk_number << "ALREADY EXISTS!";
                freeChunk(pChunk);
            } else {
                //qDebug() << "Inserting chunk" << pChunk << pChunk->chunk_number;
                m_allocatedChunks.insert(pChunk->chunk_number, pChunk);

                // Insert the chunk into the LRU list
                m_mruChunk = insertIntoLRUList(pChunk, m_mruChunk);

                // If this chunk has no next LRU then it is the LRU. This only
                // happens if this is the first allocated chunk.
                if (pChunk->next_lru == NULL) {
                    m_lruChunk = pChunk;
                }
            }
        } else if (status.status == CHUNK_READ_EOF) {
            Chunk* pChunk = status.chunk;
            if (pChunk == NULL) {
                qDebug() << "ERROR: status.chunk is NULL in CHUNK_READ_EOF ReaderStatusUpdate. Ignoring update.";
                continue;
            }
            Chunk* pChunk2 = m_chunksBeingRead.take(pChunk->chunk_number);
            if (pChunk2 != pChunk) {
                qDebug() << "Mismatch in requested chunk to read!";
            }
            freeChunk(pChunk);
        } else if (status.status == CHUNK_READ_INVALID) {
            qDebug() << "WARNING: READER THREAD RECEIVED INVALID CHUNK READ";
            Chunk* pChunk = status.chunk;
            if (pChunk == NULL) {
                qDebug() << "ERROR: status.chunk is NULL in CHUNK_READ_INVALID ReaderStatusUpdate. Ignoring update.";
                continue;
            }
            Chunk* pChunk2 = m_chunksBeingRead.take(pChunk->chunk_number);
            if (pChunk2 != pChunk) {
                qDebug() << "Mismatch in requested chunk to read!";
            }
            freeChunk(pChunk);
        }
    }
}

int CachingReader::read(int sample, int num_samples, CSAMPLE* buffer) {
    // Check for bad inputs
    if (sample % 2 != 0 || num_samples < 0 || !buffer) {
        QString temp = QString("Sample = %1").arg(sample);
        QByteArray tempBA = QString(temp).toUtf8();
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

    // TODO: is it possible to move this code out of caching reader
    // and into enginebuffer?  It doesn't quite make sense here, although
    // it makes preroll completely transparent to the rest of the code

    //if we're in preroll...
    int zerosWritten = 0;
    if (sample < 0) {
        if (sample + num_samples <= 0) {
            //everything is zeros, easy
            memset(buffer, 0, sizeof(*buffer) * num_samples);
            return num_samples;
        } else {
            //some of the buffer is zeros, some is from the file
            memset(buffer, 0, sizeof(*buffer) * (0 - sample));
            buffer += (0 - sample);
            num_samples = sample + num_samples;
            zerosWritten = (0 - sample);
            sample = 0;
            //continue processing the rest of the chunks normally
        }
    }

    int start_sample = math_min(m_iTrackNumSamplesCallbackSafe,
                                sample);
    int start_chunk = chunkForSample(start_sample);
    int end_sample = math_min(m_iTrackNumSamplesCallbackSafe,
                              sample + num_samples - 1);
    int end_chunk = chunkForSample(end_sample);

    int samples_remaining = num_samples;
    int current_sample = sample;

    // Sanity checks
    if (start_chunk > end_chunk) {
        qDebug() << "CachingReader::read() bad chunk range to read ["
                 << start_chunk << end_chunk << "]";
        return 0;
    }

    for (int chunk_num = start_chunk; chunk_num <= end_chunk; chunk_num++) {
        Chunk* current = lookupChunk(chunk_num);

        // If the chunk is not in cache, then we must return an error.
        if (current == NULL) {
            qDebug() << "Couldn't get chunk " << chunk_num
                     << " in read() of [" << sample << "," << sample+num_samples
                     << "] chunks " << start_chunk << "-" << end_chunk;

            // Something is wrong. Break out of the loop, that should fill the
            // samples requested with zeroes.
            break;
        }

        int chunk_start_sample = sampleForChunk(chunk_num);
        int chunk_offset = current_sample - chunk_start_sample;
        int chunk_remaining_samples = current->length - chunk_offset;

        // More sanity checks
        if (current_sample < chunk_start_sample || current_sample % 2 != 0) {
            qDebug() << "CachingReader::read() bad chunk parameters"
                     << "chunk_start_sample" << chunk_start_sample
                     << "current_sample" << current_sample;
            break;
        }

        // If we're past the start_chunk then current_sample should be
        // chunk_start_sample.
        if (start_chunk != chunk_num && chunk_start_sample != current_sample) {
            qDebug() << "CachingReader::read() bad chunk parameters"
                     << "chunk_num" << chunk_num
                     << "start_chunk" << start_chunk
                     << "chunk_start_sample" << chunk_start_sample
                     << "current_sample" << current_sample;
            break;
        }

        if (samples_remaining < 0) {
            qDebug() << "CachingReader::read() bad samples remaining"
                     << samples_remaining;
            break;
        }

        // It is completely possible that chunk_remaining_samples is less than
        // zero. If the caller is trying to read from beyond the end of the
        // file, then this can happen. We should tolerate it.
        int samples_to_read = math_max(0, math_min(samples_remaining,
                                                   chunk_remaining_samples));

        // If we did not decide to read any samples from this chunk then that
        // means we have exhausted all the samples in the song.
        if (samples_to_read == 0) {
            break;
        }

        // samples_to_read should be non-negative and even
        if (samples_to_read < 0 || samples_to_read % 2 != 0) {
            qDebug() << "CachingReader::read() samples_to_read invalid"
                     << samples_to_read;
            break;
        }

        // TODO(rryan) do a test and see if using memcpy is faster than gcc
        // optimizing the for loop
        CSAMPLE *data = current->data + chunk_offset;
        memcpy(buffer, data, sizeof(*buffer) * samples_to_read);
        // for (int i=0; i < samples_to_read; i++) {
        //     buffer[i] = data[i];
        // }

        buffer += samples_to_read;
        current_sample += samples_to_read;
        samples_remaining -= samples_to_read;
    }

    // If we didn't supply all the samples requested, that probably means we're
    // at the end of the file, or something is wrong. Provide zeroes and pretend
    // all is well. The caller can't be bothered to check how long the file is.
    // TODO(XXX) memset
    for (int i=0; i<samples_remaining; i++) {
        buffer[i] = 0.0f;
    }
    samples_remaining = 0;

    if (samples_remaining != 0) {
        qDebug() << "CachingReader::read() did read all requested samples.";
    }
    return zerosWritten + num_samples - samples_remaining;
}

void CachingReader::hintAndMaybeWake(QList<Hint>& hintList) {
    // If no file is loaded, skip.
    if (m_readerStatus != TRACK_LOADED) {
        return;
    }

    QListIterator<Hint> iterator(hintList);

    // To prevent every bit of code having to guess how many samples
    // forward it makes sense to keep in memory, the hinter can provide
    // either 0 for a forward hint or -1 for a backward hint. We should
    // be calculating an appropriate number of samples to go backward as
    // some function of the latency, but for now just leave this as a
    // constant. 2048 is a pretty good number of samples because 25ms
    // latency corresponds to 1102.5 mono samples and we need double
    // that for stereo samples.
    const int default_samples = 2048;

    QSet<int> chunksToFreshen;
    while (iterator.hasNext()) {
        // Copy, don't use reference.
        Hint hint = iterator.next();

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
        int start_sample = math_max(0, math_min(
            m_iTrackNumSamplesCallbackSafe, hint.sample));
        int start_chunk = chunkForSample(start_sample);
        int end_sample = math_max(0, math_min(
            m_iTrackNumSamplesCallbackSafe, hint.sample + hint.length - 1));
        int end_chunk = chunkForSample(end_sample);

        for (int current = start_chunk; current <= end_chunk; ++current) {
            chunksToFreshen.insert(current);
        }
    }

    // For every chunk that the hints indicated, check if it is in the cache. If
    // any are not, then wake.
    bool shouldWake = false;
    QSetIterator<int> setIterator(chunksToFreshen);
    while (setIterator.hasNext()) {
        int chunk = setIterator.next();

        // This will cause the chunk to be 'freshened' in the cache. The
        // chunk will be moved to the end of the LRU list.
        if (!m_chunksBeingRead.contains(chunk) && lookupChunk(chunk) == NULL) {
            shouldWake = true;
            Chunk* pChunk = allocateChunkExpireLRU();
            if (pChunk == NULL) {
                qDebug() << "ERROR: Couldn't allocate spare Chunk to make ChunkReadRequest.";
                continue;
            }
            m_chunksBeingRead.insert(chunk, pChunk);
            ChunkReadRequest request;
            pChunk->chunk_number = chunk;
            request.chunk = pChunk;
            // qDebug() << "Requesting read of chunk" << chunk << "into" << pChunk;
            // qDebug() << "Requesting read into " << request.chunk->data;
            if (m_chunkReadRequestFIFO.write(&request, 1) != 1) {
                qDebug() << "ERROR: Could not submit read request for "
                         << chunk;
            }
            //qDebug() << "Checking chunk " << chunk << " shouldWake:" << shouldWake << " chunksToRead" << m_chunksToRead.size();
        }
    }

    // If there are chunks to be read, wake up.
    if (shouldWake) {
        wake();
    }
}

void CachingReader::run() {
    // Notify the EngineWorkerScheduler that the work we scheduled is starting.
    emit(workStarting(this));

    m_trackQueueMutex.lock();
    TrackPointer pLoadTrack = TrackPointer();
    if (!m_trackQueue.isEmpty()) {
        pLoadTrack = m_trackQueue.takeLast();
        m_trackQueue.clear();
    }
    m_trackQueueMutex.unlock();

    if (pLoadTrack) {
        loadTrack(pLoadTrack);
    } else {
        // Read the requested chunks.
        ChunkReadRequest request;
        ReaderStatusUpdate status;
        while (m_chunkReadRequestFIFO.read(&request, 1) == 1) {
            processChunkReadRequest(&request, &status);
            m_readerStatusFIFO.writeBlocking(&status, 1);
        }
    }

    // Notify the EngineWorkerScheduler that the work we did is done.
    emit(workDone(this));
}

void CachingReader::wake() {
    //qDebug() << m_pGroup << "CachingReader::wake()";
    emit(workReady(this));
}

void CachingReader::loadTrack(TrackPointer pTrack) {
    //qDebug() << m_pGroup << "CachingReader::loadTrack() lock acquired for load.";

    ReaderStatusUpdate status;
    status.status = TRACK_LOADED;
    status.chunk = NULL;
    status.trackNumSamples = 0;

    if (m_pCurrentSoundSource != NULL) {
        delete m_pCurrentSoundSource;
        m_pCurrentSoundSource = NULL;
    }
    m_iTrackSampleRate = 0;
    m_iTrackNumSamples = 0;

    QString filename = pTrack->getLocation();

    if (filename.isEmpty() || !pTrack->exists()) {
        // Must unlock before emitting to avoid deadlock
        qDebug() << m_pGroup << "CachingReader::loadTrack() load failed for\""
                 << filename << "\", unlocked reader lock";
        status.status = TRACK_NOT_LOADED;
        m_readerStatusFIFO.writeBlocking(&status, 1);
        emit(trackLoadFailed(
            pTrack, QString("The file '%1' could not be found.").arg(filename)));
        return;
    }

    m_pCurrentSoundSource = new SoundSourceProxy(pTrack);
    bool openSucceeded = (m_pCurrentSoundSource->open() == OK); //Open the song for reading
    m_iTrackSampleRate = m_pCurrentSoundSource->getSampleRate();
    m_iTrackNumSamples = status.trackNumSamples =
            m_pCurrentSoundSource->length();

    if (!openSucceeded || m_iTrackNumSamples == 0 || m_iTrackSampleRate == 0) {
        // Must unlock before emitting to avoid deadlock
        qDebug() << m_pGroup << "CachingReader::loadTrack() load failed for\""
                 << filename << "\", file invalid, unlocked reader lock";
        status.status = TRACK_NOT_LOADED;
        m_readerStatusFIFO.writeBlocking(&status, 1);
        emit(trackLoadFailed(
            pTrack, QString("The file '%1' could not be loaded.").arg(filename)));
        return;
    }

    m_readerStatusFIFO.writeBlocking(&status, 1);

    // Clear the chunks to read list.
    ChunkReadRequest request;
    while (m_chunkReadRequestFIFO.read(&request, 1) == 1) {
        qDebug() << "Skipping read request for " << request.chunk->chunk_number;
        status.status = CHUNK_READ_INVALID;
        status.chunk = request.chunk;
        m_readerStatusFIFO.writeBlocking(&status, 1);
    }

    // Emit that the track is loaded.
    emit(trackLoaded(pTrack, m_iTrackSampleRate, m_iTrackNumSamples));
}
