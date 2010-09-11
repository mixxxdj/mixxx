
#include <math.h>

#include <QtDebug>
#include <QFileInfo>

#include "controlobject.h"
#include "controlobjectthread.h"

#include "cachingreader.h"
#include "trackinfoobject.h"
#include "soundsourceproxy.h"


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
    m_pCurrentTrack(),
    m_pCurrentSoundSource(NULL),
    m_iTrackSampleRate(0),
    m_iTrackNumSamples(0),
    m_mruChunk(NULL),
    m_lruChunk(NULL),
    m_pRawMemoryBuffer(NULL),
    m_iRawMemoryBufferLength(0),
    m_bQuit(false) {
    initialize();
}

CachingReader::~CachingReader() {

    m_readerMutex.lock();
    m_freeChunks.clear();
    m_allocatedChunks.clear();
    m_lruChunk = m_mruChunk = NULL;

    delete [] m_pSample;

    delete [] m_pRawMemoryBuffer;
    m_pRawMemoryBuffer = NULL;
    m_iRawMemoryBufferLength = 0;

    m_readerMutex.unlock();
}

void CachingReader::initialize() {
    int memory_to_use = 5000000; // 5mb, TODO

    Q_ASSERT(memory_to_use >= kChunkLength);

    // Only allocate as many bytes as we will actually use.
    memory_to_use -= (memory_to_use % kChunkLength);

    m_pSample = new SAMPLE[kSamplesPerChunk];

    Q_ASSERT(kSamplesPerChunk * sizeof(CSAMPLE) == kChunkLength);

    int total_chunks = memory_to_use / kChunkLength;

    qDebug() << "CachingReader using" << memory_to_use << "bytes.";

    m_iRawMemoryBufferLength = kSamplesPerChunk * total_chunks;
    m_pRawMemoryBuffer = new CSAMPLE[m_iRawMemoryBufferLength];

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
    Q_ASSERT(chunk);

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
    Q_ASSERT(chunk);

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
    Q_ASSERT(removed <= 1);

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

    for (int i=0; i < m_chunks.size(); i++) {
        Chunk* c = m_chunks[i];
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
        Q_ASSERT(m_lruChunk);
        //qDebug() << "Expiring LRU" << m_lruChunk->chunk_number;
        freeChunk(m_lruChunk);
        chunk = allocateChunk();
        Q_ASSERT(chunk);
    }
    return chunk;
}

Chunk* CachingReader::lookupChunk(int chunk_number) {
    // Defaults to NULL if it's not in the hash.
    Chunk* chunk = NULL;

    if (m_allocatedChunks.contains(chunk_number)) {
        chunk = m_allocatedChunks.value(chunk_number);

        // Make sure we're all in agreement here.
        Q_ASSERT(chunk_number == chunk->chunk_number);

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

Chunk* CachingReader::getChunk(int chunk_number, bool* cache_miss) {
    Chunk* chunk = lookupChunk(chunk_number);

    if (cache_miss != NULL)
        *cache_miss = (chunk == NULL);

    // If it wasn't in the cache, read it from file.
    if (chunk == NULL) {
        //qDebug() << "Cache miss on chunk " << chunk_number;
        chunk = allocateChunkExpireLRU();
        Q_ASSERT(chunk != NULL);

        if (!readChunkFromFile(chunk, chunk_number)) {
            //qDebug() << "Failed to read chunk " << chunk_number;
            freeChunk(chunk);
            return NULL;
        } else {
            Q_ASSERT(chunk_number == chunk->chunk_number);
            m_allocatedChunks.insert(chunk_number, chunk);

            // Insert the chunk into the LRU list
            m_mruChunk = insertIntoLRUList(chunk, m_mruChunk);

            // If this chunk has no next LRU then it is the LRU. This only
            // happens if this is the first allocated chunk.
            if (chunk->next_lru == NULL) {
                m_lruChunk = chunk;
            }

        }
    }

    return chunk;
}

bool CachingReader::readChunkFromFile(Chunk* pChunk, int chunk_number) {

    if (m_pCurrentSoundSource == NULL || pChunk == NULL || chunk_number < 0)
        return false;

    // Stereo samples
    int sample_position = sampleForChunk(chunk_number);
    int samples_remaining = m_iTrackNumSamples - sample_position;
    int samples_to_read = math_min(kSamplesPerChunk, samples_remaining);

    // Bogus chunk number
    if (samples_to_read <= 0)
        return false;

    m_pCurrentSoundSource->seek(sample_position);
    int samples_read = m_pCurrentSoundSource->read(samples_to_read,
                                                   m_pSample);

    //If we've run out of music, the SoundSource can return 0 samples.
    //Remember that SoundSourc->getLength() (which is m_iTrackNumSamples)
    //can lie to us about the length of the song!
    if (samples_read <= 0)
        return false;

    // TODO(XXX) This loop can't be done with a memcpy, but could be done with
    // SSE.
    CSAMPLE* buffer = pChunk->data;
    for (int i=0; i < samples_read; i++) {
        buffer[i] = CSAMPLE(m_pSample[i]);
    }

    pChunk->chunk_number = chunk_number;
    pChunk->length = samples_read;

    return true;
}

void CachingReader::newTrack(TrackPointer pTrack) {
    m_trackQueueMutex.lock();
    m_trackQueue.enqueue(pTrack);
    m_trackQueueMutex.unlock();
}

int CachingReader::read(int sample, int num_samples, CSAMPLE* buffer) {
    // Check for bogus sample numbers
    Q_ASSERT(sample >= 0);
    Q_ASSERT(sample % 2 == 0);
    Q_ASSERT(num_samples >= 0);

    // If asked to read 0 samples, don't do anything. (this is a perfectly
    // reasonable request that happens sometimes. If no track is loaded, don't
    // do anything.
    if (num_samples == 0 ||
        m_iTrackSampleRate == 0) {
        return 0;
    }

    int start_chunk = chunkForSample(sample);
    int end_chunk = chunkForSample(sample + num_samples - 1);

    int samples_remaining = num_samples;
    int current_sample = sample;

    // Sanity checks
    Q_ASSERT(start_chunk <= end_chunk);

    bool cache_miss = false;
    // Need to lock while we're touching Chunk's
    m_readerMutex.lock();
    for (int chunk_num = start_chunk; chunk_num <= end_chunk; chunk_num++) {

        Chunk* current = getChunk(chunk_num, &cache_miss);

        if (cache_miss) {
            //qDebug() << "Cache miss in read() on chunk" << chunk_num;
        }

        // getChunk gets a chunk at any cost. If it has failed to lookup the
        // chunk, then there is a serious issue.
        if (current == NULL) {
            qDebug() << "Couldn't get chunk " << start_chunk << " in read()";
            // Something is wrong. Break out of the loop, that should fill the
            // samples requested with zeroes.
            break;
        }

        int chunk_start_sample = sampleForChunk(chunk_num);
        int chunk_offset = current_sample - chunk_start_sample;
        int chunk_remaining_samples = current->length - chunk_offset;

        // More sanity checks
        Q_ASSERT(current_sample >= chunk_start_sample);
        Q_ASSERT(current_sample % 2 == 0);

        if (start_chunk != chunk_num) {
            Q_ASSERT(chunk_start_sample == current_sample);
        }

        Q_ASSERT(samples_remaining >= 0);
        // It is completely possible that chunk_remaining_samples is less than
        // zero. If the caller is trying to read from beyond the end of the
        // file, then this can happen. We should tolerate it.

        int samples_to_read = math_max(0, math_min(samples_remaining,
                                                   chunk_remaining_samples));

        // samples_to_read should be non-negative and even
        Q_ASSERT(samples_to_read >= 0);
        Q_ASSERT(samples_to_read % 2 == 0);

        CSAMPLE *data = current->data + chunk_offset;

        // If we did not decide to read any samples from this chunk then that
        // means we have exhausted all the samples in the song.
        if (samples_to_read == 0) {
            break;
        }

        // TODO(rryan) do a test and see if using memcpy is faster than gcc
        // optimizing the for loop
        memcpy(buffer, data, sizeof(*buffer) * samples_to_read);
        // for (int i=0; i < samples_to_read; i++) {
        //     buffer[i] = data[i];
        // }

        buffer += samples_to_read;
        current_sample += samples_to_read;
        samples_remaining -= samples_to_read;
    }
    m_readerMutex.unlock();

    // If we didn't supply all the samples requested, that probably means we're
    // at the end of the file, or something is wrong. Provide zeroes and pretend
    // all is well. The caller can't be bothered to check how long the file is.
    for (int i=0; i<samples_remaining; i++) {
        buffer[i] = 0.0f;
    }
    samples_remaining = 0;

    Q_ASSERT(samples_remaining == 0);
    return num_samples - samples_remaining;
}

void CachingReader::hint(Hint& hint) {
}

void CachingReader::hint(QList<Hint>& hintList) {
}

void CachingReader::hintAndMaybeWake(QList<Hint>& hintList) {
    QMutexLocker lock(&m_readerMutex);

    // If no file is loaded, skip.
    if (m_iTrackSampleRate == 0)
        return;

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
    while(iterator.hasNext()) {
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
        Q_ASSERT(hint.sample >= 0);
        Q_ASSERT(hint.length >= 0);
        int start_chunk = chunkForSample(hint.sample);
        int end_chunk = chunkForSample(hint.sample + hint.length);

        for (int current = start_chunk; current <= end_chunk; ++current) {
            chunksToFreshen.insert(current);
        }
    }

    // For every chunk that the hints indicated, check if it is in the cache. If
    // any are not, then wake.
    bool shouldWake = false;
    QSetIterator<int> setIterator(chunksToFreshen);
    while(setIterator.hasNext()) {
        int chunk = setIterator.next();

        // This will cause the chunk to be 'freshened' in the cache. The
        // chunk will be moved to the end of the LRU list.
        if (lookupChunk(chunk) == NULL) {
            shouldWake = true;
            m_chunksToRead.insert(chunk);
            //qDebug() << "Checking chunk " << chunk << " shouldWake:" << shouldWake << " chunksToRead" << m_chunksToRead.size();
        }

    }

    lock.unlock();
    // If there are chunks to be read, wake up.
    if (shouldWake) {
        wake();
    }
}

void CachingReader::run() {
    // Notify the EngineWorkerScheduler that the work we scheduled is starting.
    emit(workStarting());

    QList<Hint> hintList;
    TrackPointer pLoadTrack = TrackPointer();

    m_readerMutex.lock();

    m_trackQueueMutex.lock();

    if (!m_trackQueue.isEmpty()) {
        pLoadTrack = m_trackQueue.takeLast();
        m_trackQueue.clear();
    }
    m_trackQueueMutex.unlock();

    if (pLoadTrack != NULL) {
        loadTrack(pLoadTrack);
    }

    // Read the requested chunks.
    for (QSet<int>::iterator it = m_chunksToRead.begin();
         it != m_chunksToRead.end(); it++) {
        int chunk = *it;
        getChunk(chunk);
    }
    m_chunksToRead.clear();

    m_readerMutex.unlock();

    // Notify the EngineWorkerScheduler that the work we did is done.
    emit(workDone());
}

void CachingReader::wake() {
    emit(workReady());
}

void CachingReader::loadTrack(TrackPointer pTrack) {
    freeAllChunks();

    if (m_pCurrentSoundSource != NULL) {
        delete m_pCurrentSoundSource;
        m_pCurrentSoundSource = NULL;
    }
    m_iTrackSampleRate = 0;
    m_iTrackNumSamples = 0;

    QString filename = pTrack->getLocation();

    if (filename.isEmpty() || !pTrack->exists()) {
        qDebug() << "Couldn't load track with filename: " << filename;
        emit(trackLoadFailed(
            pTrack,
            QString("The file '%1' could not be found.").arg(filename)));
        return;
    }

    m_pCurrentSoundSource = new SoundSourceProxy(pTrack);
    m_pCurrentSoundSource->open(); //Open the song for reading
    m_pCurrentTrack = pTrack;
    m_iTrackSampleRate = m_pCurrentSoundSource->getSampleRate();
    m_iTrackNumSamples = m_pCurrentSoundSource->length();

    if (m_iTrackNumSamples == 0 || m_iTrackSampleRate == 0) {
        qDebug() << "Track is invalid: " << filename;
        emit(trackLoadFailed(
            pTrack,
            QString("The file '%1' could not be loaded.").arg(filename)));
        return;
    }

    // Clear the chunks to read list.
    m_chunksToRead.clear();

    // Emit that the track is loaded.
    emit(trackLoaded(pTrack, m_iTrackSampleRate, m_iTrackNumSamples));
}


int CachingReader::getTrackSampleRate() {
    m_readerMutex.lock();
    int value = m_iTrackSampleRate;
    m_readerMutex.unlock();
    return value;
}

int CachingReader::getTrackNumSamples() {
    m_readerMutex.lock();
    int value = m_iTrackNumSamples;
    m_readerMutex.unlock();
    return value;
}

