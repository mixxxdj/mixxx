#ifndef ANALYSERQUEUE_H
#define ANALYSERQUEUE_H

#include <QList>
#include <QThread>
#include <QQueue>
#include <QWaitCondition>

#include "analyser.h"
#include "trackinfoobject.h"

class AnalyserQueue : public QThread {

public:
	AnalyserQueue();
	virtual ~AnalyserQueue();


	void queueAnalyseTrack(TrackInfoObject* tio);

	void run();

	static AnalyserQueue* createDefaultAnalyserQueue();

private:
	void addAnalyser(Analyser* an);

	QList<Analyser*> m_aq;

	TrackInfoObject* dequeueNextBlocking();
	void doAnalysis(TrackInfoObject* tio);

	bool m_exit;

	// The processing queue and associated mutex
	QQueue<TrackInfoObject*> m_tioq;
	QMutex m_qm;
	QWaitCondition m_qwait;
};

#endif
