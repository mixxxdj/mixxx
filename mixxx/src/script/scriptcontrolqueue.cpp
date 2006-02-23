#include "scriptcontrolqueue.h"
#include "numbercontrolevent.h"
#include "trackcontrolevent.h"

ScriptControlQueue::ScriptControlQueue(ScriptEngine* parent) : QObject() {
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(timerCallback()));

	m_parent = parent;
}

ScriptControlQueue::~ScriptControlQueue() {
}

void ScriptControlQueue::setupCallbacks() {
	//qDebug("setupCallbacks()");
	if (m_timer.isActive()) {
		m_timer.stop();
	}
	//qDebug("Test 1");
	if (m_q.isEmpty()) {
		//qDebug("Queue is empty");
		return;
	}
	//qDebug("Test 2");
	ScriptControlEvent* ev = m_q.first();
	//qDebug("Test 3");
	const QDateTime* evtime = ev->getTime();
	//qDebug("Test 4");
	QDateTime now = QDateTime::currentDateTime();
	//qDebug("Test 5");
	int delta = 0;
	//qDebug("evtime: %x", evtime);
	//qDebug("Event: %i", evtime->toTime_t());
	//qDebug("Now: %i", now.toTime_t());
	
	if (*evtime <= now) {
		// Theres an event on the queue which we should be executing now
		//qDebug("Test: %i %i", evtime->time().msec(), now.time().msec());
		//qDebug("Test: %f", ev->getValue());
		m_q.removeFirst();
		//qDebug("Ptr: %x", ev);
		ev->execute();
		delete ev;
	} else {
		// Theres an event, but not for a while so sleep until then
		delta = now.secsTo(*evtime) * 1000;
		int msecsnow = now.time().msec();
		int msecsev = evtime->time().msec();
		//qDebug("msecsnow %i", msecsnow);
		//qDebug("msecsev %i", msecsev);
		if (msecsev > msecsnow) {
			delta += msecsev - msecsnow;
		} else if (msecsev < msecsnow) {
			delta -= (msecsnow - msecsev);
		}
		// Try to compensate for the fact the triggering the event takes
		// a few ms
		//delta -= 10;
		
		// Never sleep for 0ms, as that means just return immediately
		// Sleep for 1ms which will give us some minimum delay
		if (delta < 1) { delta = 1; }
	}
	
	// Ring me back when it's time to process another event
	//qDebug("Sleep time %i", delta);
	//qDebug("Event: %i", evtime->toTime_t());
	m_timer.start(delta, TRUE);
}

void ScriptControlQueue::timerCallback() {
	//qDebug("timerCallback()");
	setupCallbacks();
}

// This is probably turbo-unthread-safe because of the way it uses qptrlist
// But it's only running in one thread at a time for now, so it's ok
void ScriptControlQueue::schedule(ScriptControlEvent *event) {
	const QDateTime* newtime = event->getTime();
	//qDebug("----");
	//qDebug(QDateTime::currentDateTime().toString());
	//qDebug(newtime->toString());
	ScriptControlEvent* ev = m_q.first();
	while(ev != 0) {
		const QDateTime* evtime = ev->getTime();
		//qDebug(evtime->toString());
		if (*newtime < *evtime) {
			//qDebug("%s:%i < %s:%i", (const char*)newtime->toString(), newtime->time().msec(), (const char*)evtime->toString(), evtime->time().msec());
			//if (newtime->time() < evtime->time()) {
			//	qDebug("Yep");
			//}
			m_q.insert(m_q.at(), event);
			return;
		}
			//qDebug("%s >= %s", (const char*)newtime->toString(), (const char*)evtime->toString());
		ev = m_q.next();
	}
	// If we got to the end, it's last
	m_q.append(event);
	//qDebug("Event is at %i in queue", m_q.findRef(event));
	m_timer.start(0, TRUE);
}

QDateTime ScriptControlQueue::getWhen(const QDateTime* base, int offset) {
	int days = 0;
	//qDebug(base->toString());
	//qDebug("+ %i", offset);
	if (base->time().addMSecs(offset) < base->time()) {
		days = 1;
	}

        QDateTime when(base->date().addDays(days), base->time().addMSecs(offset));
	return when;
}


void ScriptControlQueue::schedule(int channel, QString path, 
		QDateTime base, int offset, int process, int tag) {
	TrackControlEvent* ev = new TrackControlEvent(m_parent, channel, path,\
			getWhen(&base, offset), process, tag);
	schedule(ev);
}

void ScriptControlQueue::schedule(const char* group, const char* name, \
		double value, const QDateTime *base, int offset, \
		int process, int tag) {
	// This is wierd code, that's because QDateTime does wierd things, like 
	// deallocate memory you're still using
	
	// TODO: For now this doesn't support macros longer than 1 day 
	// (not an immediate problem :)
	//qDebug("+%i->%i:%f", offset, when, (float)value);
	schedule(new NumberControlEvent(group, name, value, \
				getWhen(base, offset), process, tag));
}

void ScriptControlQueue::interpolate(const char* group, const char* name, \
		const QDateTime *base, int time1, double val1, int time2, \
		double val2, int process, int tag, bool addLast, int minres) {
	
	//qDebug("%i %f -> %i %f", time1, val1, time2, val2);
			
	int diff = time2 - time1;
	double dv = val2 - val1;
	int chunks = ((diff) / minres);
	if ((chunks * minres) < diff) { chunks++; }
	int max = chunks;
	if (addLast) { max++; }
	
	for (int i = 0; i < max; i++) {
		int time = ((diff*i)/chunks) + time1;
		//qDebug("At time %i", time);
		//qDebug("%f", ((double)i/(double)max));
		double val = val1 + (((double)i/(double)chunks) * dv);
		schedule(group, name, val, base, time, process, tag);
		//qDebug("Schedule at %i => %f", time, val);
		//qDebug("Value %f", val);
	}
}

void ScriptControlQueue::killProcess(int process) {
	ScriptControlEvent *ev = m_q.first();
	while (ev != 0) {
		if (ev->getProcess() == process) {
			m_q.remove();
			delete ev;
			ev = m_q.current();
		} else {
			ev = m_q.next();
		}
	}
}

void ScriptControlQueue::killTag(int process, int tag) {
	ScriptControlEvent *ev = m_q.first();
	while (ev != 0) {
		if (ev->getProcess() == process && ev->getTag() == tag) {
			m_q.remove();
			delete ev;
			ev = m_q.current();
		} else {
			ev = m_q.next();
		}
	}
}
