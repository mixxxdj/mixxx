#include <QtDebug>

#include "trackinfoobject.h"
#include "analyserqueue.h"
#include "soundsourceproxy.h"

#ifdef __TONAL__
#include "tonal/tonalanalyser.h"
#endif

#include "analyserwaveform.h"
#include "analyserwavesummary.h"
#include "analyserbpm.h"

AnalyserQueue::AnalyserQueue() : m_aq(),
		m_tioq(), m_qm(), m_qwait(), m_exit(false) {

}

void AnalyserQueue::addAnalyser(Analyser* an) {
	m_aq.push_back(an);
}

TrackInfoObject* AnalyserQueue::dequeueNextBlocking() {
		m_qm.lock();

		if (m_tioq.isEmpty()) {
			m_qwait.wait(&m_qm);
		}

		TrackInfoObject* tio = m_tioq.dequeue();

		m_qm.unlock();

		Q_ASSERT(tio != NULL);

		return tio;
}

void AnalyserQueue::doAnalysis(TrackInfoObject* tio, SoundSourceProxy *pSoundSource) {

	// CHANGING THIS WILL BREAK TONALANALYSER!!!!
	const int ANALYSISBLOCKSIZE = 2*32768;

	SAMPLE data16[ANALYSISBLOCKSIZE];
    CSAMPLE samples[ANALYSISBLOCKSIZE];

	int read = 0;

    // FIXME: Duplicate mono stuff to stereo for analysis
	//Q_ASSERT(tio->getChannels() == 2);
	if (tio->getChannels() != 2) {
        qDebug() << "FIXME: Engine analysis requires songs to be in stereo.";
        return;
    }
    
    do {
		read = pSoundSource->read(ANALYSISBLOCKSIZE, data16);

		// Safety net in case something later barfs on 0 sample input
		if (read == 0) {
			break;
		}


		for (int i = 0; i < read; i++) {
			samples[i] = ((float)data16[i])/32767.0f;
		}

		QListIterator<Analyser*> it(m_aq);

		while (it.hasNext()) {
			it.next()->process(samples, read);
		}

	} while(read == ANALYSISBLOCKSIZE);

}

void AnalyserQueue::run() {

	while (!m_exit) {

		TrackInfoObject* next = dequeueNextBlocking();
        
        // Get the audio
        SoundSourceProxy * pSoundSource = new SoundSourceProxy(next);
        int iNumSamples = pSoundSource->length();
        int iSampleRate = pSoundSource->getSrate();

		QListIterator<Analyser*> it(m_aq);

		while (it.hasNext()) {
			it.next()->initialise(next, iSampleRate, iNumSamples);
		}

		doAnalysis(next, pSoundSource);

		QListIterator<Analyser*> itf(m_aq);

		while (itf.hasNext()) {
			itf.next()->finalise(next);
		}

	}
}

void AnalyserQueue::queueAnalyseTrack(TrackInfoObject* tio) {
	m_qm.lock();
	m_tioq.enqueue(tio);
	m_qwait.wakeAll();
	m_qm.unlock();
}

AnalyserQueue* AnalyserQueue::createDefaultAnalyserQueue(ConfigObject<ConfigValue> *_config) {
	AnalyserQueue* ret = new AnalyserQueue();

#ifdef __TONAL__
	ret->addAnalyser(new TonalAnalyser());
#endif
    
    ret->addAnalyser(new AnalyserWavesummary());
    ret->addAnalyser(new AnalyserWaveform());
    ret->addAnalyser(new AnalyserBPM(_config));

	ret->start(QThread::IdlePriority);
	return ret;
}

AnalyserQueue::~AnalyserQueue() {
		QListIterator<Analyser*> it(m_aq);

		while (it.hasNext()) {
			delete it.next();
		}
}
