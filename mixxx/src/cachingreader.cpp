
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
    m_pCurrentTrack(NULL),
    m_pTrackEnd(NULL),
    m_pCurrentSoundSource(NULL),
    m_iTrackSampleRate(0),
    m_iTrackNumSamples(0),
    m_pRawMemoryBuffer(NULL),
    m_iRawMemoryBufferLength(0),
    m_bQuit(false) {
    m_pTrackEnd = new ControlObjectThread(ControlObject::getControl(ConfigKey(_group,"TrackEnd")));
    initialize();

}

CachingReader::~CachingReader() {

    stop();
    m_readerMutex.lock();
    m_freeChunks.clear();
    m_allocatedChunks.clear();
    m_recentlyUsedChunks.clear();

    delete [] m_pSample;

    delete [] m_pRawMemoryBuffer;
    m_pRawMemoryBuffer = NULL;
    m_iRawMemoryBufferLength = 0;

    delete m_pTrackEnd;
    m_pTrackEnd = NULL;

    m_readerMutex.unlock();
}

void CachingReader::initialize() {
    int memory_to_use = 5000000; // 5mb, TODO

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

    for (int i=0; i < total_chunks; i++) {
        Chunk* c = new Chunk;
        c->chunk_number = -1;
        c->length = 0;
        c->data = bufferStart;

        m_chunks.push_back(c);
        m_freeChunks.push_back(c);

        bufferStart += kSamplesPerChunk;
    }
}


void CachingReader::freeChunk(Chunk* pChunk) {
    int removed = m_allocatedChunks.remove(pChunk->chunk_number);

    // We'll tolerate not being in allocatedChunks because sometime you free a
    // chunk right after you allocated it.
    Q_ASSERT(removed <= 1);

    removed = m_recentlyUsedChunks.removeAll(pChunk);
    Q_ASSERT(removed <= 1);

    pChunk->chunk_number = -1;
    pChunk->length = 0;
    m_freeChunks.push_back(pChunk);
}

void CachingReader::freeAllChunks() {
    m_allocatedChunks.clear();
    m_recentlyUsedChunks.clear();

    for (int i=0; i < m_chunks.size(); i++) {
        Chunk* c = m_chunks[i];
        if (!m_freeChunks.contains(c)) {
            c->chunk_number = -1;
            c->length = 0;
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
        freeChunk(m_recentlyUsedChunks.last());
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

        int times = m_recentlyUsedChunks.removeAll(chunk);

        if (times != 1) {
            qDebug() << "WARNING: chunk" << chunk_number
                     << "was in recently used list" << times << "times";
        }

        m_recentlyUsedChunks.push_front(chunk);
    }

    return chunk;
}

Chunk* CachingReader::getChunk(int chunk_number) {

    Chunk* chunk = lookupChunk(chunk_number);

    if (chunk == NULL) {
        //qDebug() << "Cache miss on chunk " << chunk_number;
        chunk = allocateChunkExpireLRU();
        Q_ASSERT(chunk != NULL);

        if (!readChunkFromFile(chunk, chunk_number)) {
            qDebug() << "Failed to read chunk " << chunk_number;
            freeChunk(chunk);
            return NULL;
        } else {
            Q_ASSERT(chunk_number == chunk->chunk_number);
            m_allocatedChunks.insert(chunk_number, chunk);
            Q_ASSERT(!m_recentlyUsedChunks.contains(chunk));
            m_recentlyUsedChunks.push_front(chunk);
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

void CachingReader::newTrack(TrackInfoObject* pTrack) {
    m_trackQueueMutex.lock();
    m_trackQueue.enqueue(pTrack);
    m_trackQueueMutex.unlock();
}

int CachingReader::read(int sample, int num_samples, CSAMPLE* buffer) {
    // Check for bogus sample numbers
    Q_ASSERT(sample >= 0);
    Q_ASSERT(num_samples >= 0);

    // If asked to read 0 samples, don't do anything. (this is a perfectly
    // reasonable request that happens sometimes.
    if (num_samples == 0) {
        return 0;
    }

    int start_chunk = chunkForSample(sample);
    int end_chunk = chunkForSample(sample + num_samples - 1);

    int samples_remaining = num_samples;
    int current_sample = sample;

    // Sanity checks
    Q_ASSERT(start_chunk <= end_chunk);

    // Need to lock while we're touching Chunk's
    m_readerMutex.lock();
    for (int chunk_num = start_chunk; chunk_num <= end_chunk; chunk_num++) {
        Chunk* current = getChunk(chunk_num);

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

void CachingReader::hint(int sample, int length, int priority) {
    m_hintQueueMutex.lock();
    Hint hint;
    hint.sample = sample;
    hint.length = length;
    hint.priority = priority;
    m_hintQueue.enqueue(hint);
    m_hintQueueMutex.unlock();
}

void CachingReader::run() {
    //XXX copypasta (should factor this out somehow), -kousu 2/2009
    unsigned static id = 0; //the id of this thread, for debugging purposes
    QThread::currentThread()->setObjectName(QString("Reader %1").arg(++id));

    QList<Hint> hintList;
    TrackInfoObject* pLoadTrack = NULL;
    while (!m_bQuit) {

        m_readerMutex.lock();
        m_readerWait.wait(&m_readerMutex);

        // TODO make this better
        if (m_bQuit) {
            m_readerMutex.unlock();
            break;
        }

        m_trackQueueMutex.lock();
        pLoadTrack = NULL;
        if (!m_trackQueue.isEmpty()) {
            pLoadTrack = m_trackQueue.takeLast();
            m_trackQueue.clear();
        }
        m_trackQueueMutex.unlock();

        if (pLoadTrack != NULL) {
            loadTrack(pLoadTrack);
        }

        hintList.clear();
        m_hintQueueMutex.lock();
        while (!m_hintQueue.isEmpty()) {
            hintList.push_back(m_hintQueue.takeFirst());
        }
        m_hintQueueMutex.unlock();

        while (!hintList.isEmpty()) {
            Hint hint = hintList.takeLast();
            int start_chunk = chunkForSample(hint.sample);
            int end_chunk = chunkForSample(hint.sample + hint.length);

            for (int current = start_chunk; current <= end_chunk; current++) {
                // This will ensure the chunk is in the cache.
                getChunk(current);
            }
        }

        m_readerMutex.unlock();
    }
}

void CachingReader::wake() {
    m_readerWait.wakeAll();
}

void CachingReader::stop() {
    m_bQuit = true;
    wake();
    wait();
}

void CachingReader::loadTrack(TrackInfoObject *pTrack) {
    freeAllChunks();

    if (m_pCurrentSoundSource != NULL) {
        delete m_pCurrentSoundSource;
        m_pCurrentSoundSource = NULL;
    }

    QString filename = pTrack->getLocation();
    QFileInfo fileInfo(filename);

    if (filename.isEmpty() || !fileInfo.exists()) {
        qDebug() << "Couldn't load track with filename: " << filename;
        return;
    }

    m_pCurrentSoundSource = new SoundSourceProxy(pTrack);
    m_pCurrentTrack = pTrack;
    m_iTrackSampleRate = m_pCurrentSoundSource->getSrate();
    m_iTrackNumSamples = m_pCurrentSoundSource->length();

    // Emit that the track is loaded.
    emit(trackLoaded(pTrack, m_iTrackSampleRate, m_iTrackNumSamples));

    // TODO(rryan) Without this, we get a nasty reader deadlock. The reader
    // should not have to handle this. Somebody should listen to the signal
    // above and set this.
    if (m_pTrackEnd) {
        m_pTrackEnd->slotSet(0.);
    }
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

