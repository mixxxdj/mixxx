
#include <math.h>

#include <QtDebug>
#include <QFileInfo>

#include "controlobject.h"
#include "controlobjectthread.h"

#include "cachingreaderworker.h"
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

const int CachingReaderWorker::kChunkLength = CHUNK_LENGTH;
const int CachingReaderWorker::kSamplesPerChunk = CHUNK_LENGTH / sizeof(CSAMPLE);



CachingReaderWorker::CachingReaderWorker(const char* group,
        FIFO<ChunkReadRequest>* pChunkReadRequestFIFO,
        FIFO<ReaderStatusUpdate>* pReaderStatusFIFO)
        : m_pGroup(group),
          m_pChunkReadRequestFIFO(pChunkReadRequestFIFO),
          m_pReaderStatusFIFO(pReaderStatusFIFO),
          m_pCurrentSoundSource(NULL),
          m_iTrackSampleRate(0),
          m_iTrackNumSamples(0),
          m_pSample(NULL) {
    int memory_to_use = 5000000; // 5mb, TODO
    Q_ASSERT(memory_to_use >= kChunkLength);

    // Only allocate as many bytes as we will actually use.
    memory_to_use -= (memory_to_use % kChunkLength);

    m_pSample = new SAMPLE[kSamplesPerChunk];
}

CachingReaderWorker::~CachingReaderWorker() {
    m_mutexRun.lock(); // do not delete if a run is still scheduled;
                       // no need for unlock, because object is gone after

    delete [] m_pSample;

    delete m_pCurrentSoundSource;
}

void CachingReaderWorker::processChunkReadRequest(ChunkReadRequest* request,
                                            ReaderStatusUpdate* update) {
    int chunk_number = request->chunk->chunk_number;
    //qDebug() << "Processing ChunkReadRequest for" << chunk_number;
    update->chunk = request->chunk;
    update->chunk->length = 0;

    if (m_pCurrentSoundSource == NULL || chunk_number < 0) {
        update->status = CHUNK_READ_INVALID;
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

// WARNING: Always called from the other thread
void CachingReaderWorker::newTrack(TrackPointer pTrack) {
    m_newTrackMutex.lock();
    m_newTrack = pTrack;
    m_newTrackMutex.unlock();
}

void CachingReaderWorker::run() {
    TrackPointer pLoadTrack;

    m_newTrackMutex.lock();
    if (m_newTrack) {
        pLoadTrack = m_newTrack;
        m_newTrack = TrackPointer();
    }
    m_newTrackMutex.unlock();

    if (pLoadTrack) {
        loadTrack(pLoadTrack);
    } else {
        // Read the requested chunks.
        ChunkReadRequest request;
        ReaderStatusUpdate status;
        while (m_pChunkReadRequestFIFO->read(&request, 1) == 1) {
            processChunkReadRequest(&request, &status);
            m_pReaderStatusFIFO->writeBlocking(&status, 1);
        }
    }
    m_mutexRun.unlock(); // Notify that the work we did is done.
}

// WARNING: Always called from other thread
void CachingReaderWorker::wake() {
    //qDebug() << m_pGroup << "CachingReaderWorker::wake()";
    if(m_mutexRun.tryLock()) {
        // tryLock succeeds if not already scheduled and not under destruction
        if(!workReady()){
            // not scheduled
            m_mutexRun.unlock();
        }
    }
}

void CachingReaderWorker::loadTrack(TrackPointer pTrack) {
    //qDebug() << m_pGroup << "CachingReaderWorker::loadTrack() lock acquired for load.";

    // Emit that a new track is loading, stops the current track
    emit(trackLoading());

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
        qDebug() << m_pGroup << "CachingReaderWorker::loadTrack() load failed for\""
                 << filename << "\", unlocked reader lock";
        status.status = TRACK_NOT_LOADED;
        m_pReaderStatusFIFO->writeBlocking(&status, 1);
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
        qDebug() << m_pGroup << "CachingReaderWorker::loadTrack() load failed for\""
                 << filename << "\", file invalid, unlocked reader lock";
        status.status = TRACK_NOT_LOADED;
        m_pReaderStatusFIFO->writeBlocking(&status, 1);
        emit(trackLoadFailed(
            pTrack, QString("The file '%1' could not be loaded.").arg(filename)));
        return;
    }

    m_pReaderStatusFIFO->writeBlocking(&status, 1);

    // Clear the chunks to read list.
    ChunkReadRequest request;
    while (m_pChunkReadRequestFIFO->read(&request, 1) == 1) {
        qDebug() << "Skipping read request for " << request.chunk->chunk_number;
        status.status = CHUNK_READ_INVALID;
        status.chunk = request.chunk;
        m_pReaderStatusFIFO->writeBlocking(&status, 1);
    }

    // Emit that the track is loaded.
    emit(trackLoaded(pTrack, m_iTrackSampleRate, m_iTrackNumSamples));
}
