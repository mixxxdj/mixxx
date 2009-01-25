#include "engineanalyserqueue.h"
#include "soundsourceproxy.h"

#ifdef __TONAL__
#include "tonal/tonalanalyser.h"
#endif

EngineAnalyserQueue::EngineAnalyserQueue() : m_aq(),
		m_tioq(), m_qm(), m_qwait(), m_exit(false) {

}

void EngineAnalyserQueue::addAnalyser(EngineAnalyser* an) {
	m_aq.push_back(an);
}

TrackInfoObject* EngineAnalyserQueue::dequeueNextBlocking() {
		m_qm.lock();

		if (m_tioq.isEmpty()) {
			m_qwait.wait(&m_qm);
		}

		TrackInfoObject* tio = m_tioq.dequeue();

		m_qm.unlock();

		Q_ASSERT(tio != NULL);

		return tio;
}

void EngineAnalyserQueue::doAnalysis(TrackInfoObject* tio) {

	// CHANGING THIS WILL BREAK TONALANALYSER!!!!
	const int ANALYSISBLOCKSIZE = 2*32768;

	SAMPLE data16[ANALYSISBLOCKSIZE];
    CSAMPLE samples[ANALYSISBLOCKSIZE];

	// Get the audio
	SoundSourceProxy * pSoundSource = new SoundSourceProxy(tio);

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

		QListIterator<EngineAnalyser*> it(m_aq);

		while (it.hasNext()) {
			it.next()->process(samples, read);
		}

	} while(read == ANALYSISBLOCKSIZE);

}

void EngineAnalyserQueue::run() {

	while (!m_exit) {

		TrackInfoObject* next = dequeueNextBlocking();

		QListIterator<EngineAnalyser*> it(m_aq);

		while (it.hasNext()) {
			it.next()->initialise(next);
		}

		doAnalysis(next);

		QListIterator<EngineAnalyser*> itf(m_aq);

		while (itf.hasNext()) {
			itf.next()->finalise(next);
		}

	}
}

void EngineAnalyserQueue::queueAnalyseTrack(TrackInfoObject* tio) {
	m_qm.lock();
	m_tioq.enqueue(tio);
	m_qwait.wakeAll();
	m_qm.unlock();
}

EngineAnalyserQueue* EngineAnalyserQueue::createDefaultAnalyserQueue() {
	EngineAnalyserQueue* ret = new EngineAnalyserQueue();

#ifdef __TONAL__
	ret->addAnalyser(new TonalAnalyser());
#endif

	ret->start(QThread::IdlePriority);
	return ret;
}

EngineAnalyserQueue::~EngineAnalyserQueue() {
		QListIterator<EngineAnalyser*> it(m_aq);

		while (it.hasNext()) {
			delete it.next();
		}
}
