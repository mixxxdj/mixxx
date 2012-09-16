#include <QtDebug>
#include <QMutexLocker>

#include "trackinfoobject.h"
#include "playerinfo.h"
#include "analyserqueue.h"
#include "soundsourceproxy.h"
#include "playerinfo.h"

#ifdef __TONAL__
#include "tonal/tonalanalyser.h"
#endif

#include "analyserwaveform.h"
#include "analyserbpm.h"
#include "analyserrg.h"
#ifdef __VAMP__
#include "analyserbeats.h"
#include "vamp/vampanalyser.h"
#endif

#include <typeinfo>

AnalyserQueue::AnalyserQueue() :
    m_aq(),
    m_exit(false),
    m_aiCheckPriorities(false),
    m_tioq(),
    m_qm(),
    m_qwait() {
}

int AnalyserQueue::numQueuedTracks()
{
    m_qm.lock();
    int numQueuedTracks = m_tioq.count();
    m_qm.unlock();
    return numQueuedTracks;
}

void AnalyserQueue::addAnalyser(Analyser* an) {
    m_aq.push_back(an);
}

bool AnalyserQueue::isLoadedTrackWaiting() {
    QMutexLocker queueLocker(&m_qm);

    const PlayerInfo& info = PlayerInfo::Instance();
    TrackPointer pTrack;
    QMutableListIterator<TrackPointer> it(m_tioq);
    while (it.hasNext()) {
        TrackPointer& pTrack = it.next();
        if (info.isTrackLoaded(pTrack)) {
            return true;
        }
    }
    return false;
}

TrackPointer AnalyserQueue::dequeueNextBlocking() {
    m_qm.lock();

    if (m_tioq.isEmpty()) {
        m_qwait.wait(&m_qm);

        if (m_exit) {
            m_qm.unlock();
            return TrackPointer();
        }
    }

    const PlayerInfo& info = PlayerInfo::Instance();
    TrackPointer pLoadTrack;
    QMutableListIterator<TrackPointer> it(m_tioq);
    while (it.hasNext()) {
        TrackPointer& pTrack = it.next();
        if (!pTrack) {
            it.remove();
            continue;
        }
        // Prioritize tracks that are loaded.
        if (info.isTrackLoaded(pTrack)) {
            qDebug() << "Prioritizing" << pTrack->getTitle() << pTrack->getLocation();
            pLoadTrack = pTrack;
            it.remove();
            break;
        }
    }

    if (!pLoadTrack && m_tioq.size() > 0) {
        pLoadTrack = m_tioq.dequeue();
    }

    m_qm.unlock();

    if (pLoadTrack) {
        qDebug() << "Analyzing" << pLoadTrack->getTitle() << pLoadTrack->getLocation();
    }
    // pTrack might be NULL, up to the caller to check.
    return pLoadTrack;
}


bool AnalyserQueue::doAnalysis(TrackPointer tio, SoundSourceProxy *pSoundSource) {

    // TonalAnalyser requires a block size of 65536. Using a different value
    // breaks the tonal analyser. We need to use a smaller block size becuase on
    // Linux, the AnalyserQueue can starve the CPU of its resources, resulting
    // in xruns.. A block size of 8192 seems to do fine.
    const int ANALYSISBLOCKSIZE = 8192;

    int totalSamples = pSoundSource->length();
    //qDebug() << tio->getFilename() << " has " << totalSamples << " samples.";
    int processedSamples = 0;

    SAMPLE *data16 = new SAMPLE[ANALYSISBLOCKSIZE];
    CSAMPLE *samples = new CSAMPLE[ANALYSISBLOCKSIZE];

    int read = 0;
    bool dieflag = false;
    bool cancelled = false;

    do {
        read = pSoundSource->read(ANALYSISBLOCKSIZE, data16);

        // Safety net in case something later barfs on 0 sample input
        if (read == 0) {
            break;
        }

        // If we get more samples than length, ask the analysers to process
        // up to the number we promised, then stop reading - AD
        if (read + processedSamples > totalSamples) {
            qDebug() << "While processing track of length " << totalSamples << " actually got "
                     << read + processedSamples << " samples, truncating analysis at expected length";
            read = totalSamples - processedSamples;
            dieflag = true;
        }

        for (int i = 0; i < read; i++) {
            samples[i] = ((float)data16[i])/32767.0f;
        }

        QListIterator<Analyser*> it(m_aq);

        while (it.hasNext()) {
            Analyser* an =  it.next();
            //qDebug() << typeid(*an).name() << ".process()";
            an->process(samples, read);
            //qDebug() << "Done " << typeid(*an).name() << ".process()";
        }

        // emit progress updates to whoever cares
        processedSamples += read;
        int progress = ((float)processedSamples)/totalSamples * 100; //fp div here prevents insano signed overflow
        emit(trackProgress(tio, progress));

        // Since this is a background analysis queue, we should co-operatively
        // yield every now and then to try and reduce CPU contention. The
        // analyser queue is CPU intensive so we want to get out of the way of
        // the audio callback thread.
        //QThread::yieldCurrentThread();
        //QThread::usleep(10);

        //has something new entered the queue?
        if (m_aiCheckPriorities)
        {
            m_aiCheckPriorities = false;
            if (! PlayerInfo::Instance().isTrackLoaded(tio) && isLoadedTrackWaiting())
            {
                qDebug() << "Interrupting analysis to give preference to a loaded track.";
                dieflag = true;
                cancelled = true;
            }
        }
    } while(read == ANALYSISBLOCKSIZE && !dieflag);

    delete[] data16;
    delete[] samples;

    return !cancelled; //don't return !dieflag or we might reanalyze over and over
}

void AnalyserQueue::stop() {
    m_exit = true;
}

void AnalyserQueue::run() {

    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("AnalyserQueue %1").arg(++id));

    // If there are no analysers, don't waste time running.
    if (m_aq.size() == 0)
        return;

    while (!m_exit) {
        TrackPointer next = dequeueNextBlocking();

        if (m_exit) //When exit is set, it makes the above unblock first.
            return;

        // If the track is NULL, get the next one. Could happen if the track was
        // queued but then deleted.
        if (!next)
            continue;

        // Get the audio
        SoundSourceProxy * pSoundSource = new SoundSourceProxy(next);
        pSoundSource->open(); //Open the file for reading
        int iNumSamples = pSoundSource->length();
        int iSampleRate = pSoundSource->getSampleRate();

        if (iNumSamples == 0 || iSampleRate == 0) {
            qDebug() << "Skipping invalid file:" << next->getLocation();
            continue;
        }

        QListIterator<Analyser*> it(m_aq);
        bool processTrack = false;
        while (it.hasNext()) {
            // Make sure not to short-circuit initialise(...)
            processTrack = it.next()->initialise(next, iSampleRate, iNumSamples) || processTrack;
        }

        if (processTrack) {
            if (!PlayerInfo::Instance().isTrackLoaded(next) && isLoadedTrackWaiting()) {
                qDebug() << "Delaying track analysis because track is not loaded -- requeuing";
                QListIterator<Analyser*> itf(m_aq);
                while (itf.hasNext()) {
                    itf.next()->cleanup(next);
                }
                queueAnalyseTrack(next);
            } else {
                bool completed = doAnalysis(next, pSoundSource);

                if (!completed) {
                    //This track was cancelled
                    QListIterator<Analyser*> itf(m_aq);
                    while (itf.hasNext()) {
                        itf.next()->cleanup(next);
                    }
                    queueAnalyseTrack(next);
                } else {
                    QListIterator<Analyser*> itf(m_aq);
                    while (itf.hasNext()) {
                        itf.next()->finalise(next);
                    }
                }
            }
        } else {
            qDebug() << "Skipping track analysis because no analyser initialized.";
        }

        delete pSoundSource;
        emit(trackFinished(next));

        m_qm.lock();
        bool empty = m_tioq.isEmpty();
        m_qm.unlock();
        if (empty) {
            emit(queueEmpty());
        }
    }
}

void AnalyserQueue::queueAnalyseTrack(TrackPointer tio) {
    m_qm.lock();
    m_aiCheckPriorities = true;
    if( !m_tioq.contains(tio)){
        m_tioq.enqueue(tio);
        m_qwait.wakeAll();
    }
    m_qm.unlock();
}

AnalyserQueue* AnalyserQueue::createAnalyserQueue(QList<Analyser*> analysers) {
    AnalyserQueue* ret = new AnalyserQueue();

    QListIterator<Analyser*> it(analysers);
    while(it.hasNext()) {
        ret->addAnalyser(it.next());
    }

    ret->start(QThread::IdlePriority);
    return ret;
}

AnalyserQueue* AnalyserQueue::createDefaultAnalyserQueue(ConfigObject<ConfigValue> *_config) {
    AnalyserQueue* ret = new AnalyserQueue();

#ifdef __TONAL__
    ret->addAnalyser(new TonalAnalyser());
#endif

    ret->addAnalyser(new AnalyserWaveform(_config));
    ret->addAnalyser(new AnalyserGain(_config));
#ifdef __VAMP__
    VampAnalyser::initializePluginPaths();
    ret->addAnalyser(new AnalyserBeats(_config));
    //ret->addAnalyser(new AnalyserVampKeyTest(_config));
#else
    ret->addAnalyser(new AnalyserBPM(_config));
#endif

    ret->start(QThread::IdlePriority);
    return ret;
}

AnalyserQueue* AnalyserQueue::createPrepareViewAnalyserQueue(ConfigObject<ConfigValue> *_config) {
    AnalyserQueue* ret = new AnalyserQueue();

    ret->addAnalyser(new AnalyserWaveform(_config));
    ret->addAnalyser(new AnalyserGain(_config));
#ifdef __VAMP__
    VampAnalyser::initializePluginPaths();
    ret->addAnalyser(new AnalyserBeats(_config));
    //ret->addAnalyser(new AnalyserVampKeyTest(_config));
#else
    ret->addAnalyser(new AnalyserBPM(_config));
#endif

    ret->start(QThread::IdlePriority);
    return ret;
}

AnalyserQueue::~AnalyserQueue() {
    QListIterator<Analyser*> it(m_aq);

    stop();

    m_qm.lock();
    m_qwait.wakeAll();
    m_qm.unlock();

    wait(); //Wait until thread has actually stopped before proceeding.

    while (it.hasNext()) {
        Analyser* an = it.next();
        //qDebug() << "AnalyserQueue: deleting " << typeid(an).name();
        delete an;
    }
}
