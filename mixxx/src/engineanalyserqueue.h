#ifndef ENGINEANALYSERQUEUE_H
#define ENGINEANALYSERQUEUE_H

#include <QList>
#include <QThread>
#include <QQueue>
#include <QWaitCondition>

#include "engineanalyser.h"
#include "trackinfoobject.h"

class EngineAnalyserQueue : public QThread {

public:
	EngineAnalyserQueue();
	virtual ~EngineAnalyserQueue();


	void queueAnalyseTrack(TrackInfoObject* tio);

	void run();

	static EngineAnalyserQueue* createDefaultAnalyserQueue();

private:
	void addAnalyser(EngineAnalyser* an);

	QList<EngineAnalyser*> m_aq;

	TrackInfoObject* dequeueNextBlocking();
	void doAnalysis(TrackInfoObject* tio);

	bool m_exit;

	// The processing queue and associated mutex
	QQueue<TrackInfoObject*> m_tioq;
	QMutex m_qm;
	QWaitCondition m_qwait;
};

#endif