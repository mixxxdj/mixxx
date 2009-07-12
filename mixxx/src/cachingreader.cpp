
#include <math.h>

#include <QFileInfo>

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

const int CachingReader::kChunkLength = CHUNK_LENGTH;
const int CachingReader::kSamplesPerChunk = CHUNK_LENGTH / sizeof(CSAMPLE);

CachingReader::CachingReader(const char* _group,
                             ConfigObject<ConfigValue>* _config) :
    m_pGroup(_group),
    m_pConfig(_config),
    m_pCurrentTrack(NULL),
    m_pCurrentSoundSource(NULL),
    m_iTrackSampleRate(0),
    m_iTrackNumSamples(0),
    m_pRawMemoryBuffer(NULL),
    m_iRawMemoryBufferLength(0),
    m_bQuit(false) {
    initialize();
}

CachingReader::~CachingReader() {

    m_readerMutex.lock();

    m_freeChunks.clear();
    m_allocatedChunks.clear();
    m_recentlyUsedChunks.clear();

    delete [] m_pSample;

    delete [] m_pRawMemoryBuffer;
    m_pRawMemoryBuffer = NULL;
    m_iRawMemoryBufferLength = 0;

    m_readerMutex.unlock();
}

void CachingReader::initialize() {
    int memory_to_use = 5000000; // 5mb, TODO

    // Only allocate as many bytes as we will actually use.
    memory_to_use -= (memory_to_use % kChunkLength);

    m_pSample = new SAMPLE[kSamplesPerChunk];

    Q_ASSERT(sizeof(char) == 1);
    m_pRawMemoryBuffer = new char[memory_to_use];

    if (m_pRawMemoryBuffer == NULL) {
        qCritical() << "Could not allocate " << memory_to_use
                 << " bytes for use in CachingReader";
    }

    m_iRawMemoryBufferLength = memory_to_use;

    int total_chunks = memory_to_use / kChunkLength;
    
    m_chunks.resize(total_chunks);
    //m_freeChunks.reserve(total_chunks);
    m_allocatedChunks.reserve(total_chunks);

    char* memory = m_pRawMemoryBuffer;

    qDebug() << "Creating total of " << total_chunks;
    
    for (int i=0; i < total_chunks; i++, memory += kChunkLength) {
        qDebug() << "Creating chunk " << i;
            
        Chunk &c = m_chunks[i];
        c.chunk_number = 0;
        c.sample = 0;
        c.length = 0;
        c.data = (CSAMPLE*)memory;
        m_freeChunks.push_back(&c);
    }
}


void CachingReader::freeChunk(Chunk* pChunk) {
    qDebug() << "freeChunk(" << pChunk->chunk_number << ")";
    Chunk* removed = m_allocatedChunks.take(pChunk->chunk_number);
    m_recentlyUsedChunks.removeOne(pChunk); // TODO
    Q_ASSERT(removed == pChunk);
    pChunk->chunk_number = -1;
    pChunk->sample = -1;
    pChunk->length = 0;
    m_freeChunks.push_back(pChunk);
}

void CachingReader::freeAllChunks() {
    m_allocatedChunks.clear();
    m_recentlyUsedChunks.clear();
    
    for (int i=0; i < m_chunks.size(); i++) {
        Chunk& c = m_chunks[i];
        c.chunk_number = 0;
        c.sample = 0;
        c.length = 0;
        m_freeChunks.push_back(&c);
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
        freeChunk(m_recentlyUsedChunks.takeLast());
        chunk = allocateChunk();
        Q_ASSERT(chunk);
    }
    return chunk;
}

Chunk* CachingReader::lookupChunk(int chunk_number) {
    // Defaults to NULL if it's not in the hash.
    Chunk* chunk = m_allocatedChunks[chunk_number];

    if (chunk != NULL) {
        m_recentlyUsedChunks.removeOne(chunk); // TODO
        m_recentlyUsedChunks.push_front(chunk);
    }

    return chunk;
}

Chunk* CachingReader::getChunk(int chunk_number) {

    Chunk* chunk = lookupChunk(chunk_number);

    if (chunk == NULL) {
        qDebug() << "Cache miss on chunk " << chunk_number;
        chunk = allocateChunkExpireLRU();
        Q_ASSERT(chunk);

        if (!readChunkFromFile(chunk, chunk_number)) {
            qDebug() << "Failed to read chunk " << chunk_number;
            freeChunk(chunk);
            return NULL;
        } else {
            m_allocatedChunks[chunk_number] = chunk;
            m_recentlyUsedChunks.push_front(chunk);
        }
    }
    
    return chunk;
}

bool CachingReader::readChunkFromFile(Chunk* pChunk, int chunk_number) {
    qDebug() << "readChunkFromFile reading chunk " << chunk_number;
    
    if (m_pCurrentSoundSource == NULL || pChunk == NULL || chunk_number < 0)
        return false;
    
    // Stereo samples
    int sample_position = chunk_number * kSamplesPerChunk;
    int samples_remaining = m_iTrackNumSamples - sample_position;
    int samples_to_read = math_min(kSamplesPerChunk, samples_remaining);

    qDebug() << "sample_position: " << sample_position
             << " samples_remaining " << samples_remaining
             << " samples_to_read " << samples_to_read;
    
    // Bogus chunk number
    if (samples_to_read <= 0)
        return false;

    m_pCurrentSoundSource->seek(sample_position);
    int samples_read = m_pCurrentSoundSource->read(samples_to_read,
                                                   m_pSample);

    for (int i=0; i < samples_read; i++) {
        if (i < 20) {
            qDebug() << "READCHUNK " << i << ":" << m_pSample[i];
        }
            
        pChunk->data[i] = CSAMPLE(m_pSample[i]);
    }
    
    pChunk->sample = sample_position;
    pChunk->length = samples_read;
    
    return true;
}

void CachingReader::newTrack(TrackInfoObject* pTrack) {
    m_trackQueueMutex.lock();
    m_trackQueue.enqueue(pTrack);
    m_trackQueueMutex.unlock();
}

int CachingReader::read(int sample, int num_samples, CSAMPLE* buffer) {

    qDebug() << "read() sample " << sample << " num_samples " << num_samples;
    int start_chunk = chunkForSample(sample);
    int end_chunk = chunkForSample(sample + num_samples);

    int samples_remaining = num_samples;

    // Need to lock while we're touching Chunk's
    m_readerMutex.lock();
    for (int chunk_num = start_chunk; chunk_num <= end_chunk; chunk_num++) {
        Chunk* current = getChunk(start_chunk);

        if (current == NULL) {
            qCritical() << "Couldn't get chunk " << start_chunk << " in read()";
        }
        int samples_to_read = math_min(samples_remaining, current->length);
        CSAMPLE* start = current->data;
        CSAMPLE* end = current->data + samples_to_read;

        qDebug() << "from chunk " << start_chunk << " reading " << samples_to_read;

        for (int i = 0; i < samples_to_read; ++i) {
            buffer[i] = start[i];
        }

        samples_remaining -= samples_to_read;
    }
    m_readerMutex.unlock();

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

