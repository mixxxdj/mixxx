#include "numberrecorder.h"
#include "../controlobjectthreadmain.h"
#include "../controlobject.h"
#include "../configobject.h"

#include <qdatetime.h>

NumberRecorder::NumberRecorder(const char* group, const char* name
		, int interp) : SignalRecorder() {
	m_group = group;
	m_name = name;
	m_interp = interp;
	m_evcount = 0;
	m_p = 0;
	
	m_times = QList<int>();
	m_values = QList<double>();
}

NumberRecorder::~NumberRecorder() {
}

void NumberRecorder::reset() {
	m_times = QList<int>();
	m_values = QList<double>();
	m_evcount = 0;

	stopRecord();
	
	// This broke a lot of stuff
	//delete m_p;

	// TODO: The SDateTime object which holds the start time is probably a
	// memory leak
}

void NumberRecorder::startRecord(SDateTime *base) {
	m_base = base;

	if (m_p == 0) {
	        ControlObject *obj = ControlObject::getControl(\
				ConfigKey(m_group, m_name));
		m_p = new ControlObjectThreadMain(obj);
		qDebug("Created cotm %x", m_p);
	}
	qDebug("Connecting...");
        connect(m_p, SIGNAL(valueChanged(double)), this, SLOT(valueCaught(double)));
        //qDebug("Started recorder for %s:%s", m_group, m_name);
}

void NumberRecorder::stopRecord() {
	if (!m_p) { return; }
	qDebug("Stopping %s:%s", m_group, m_name);
	m_p->disconnect(this);
	qDebug("Stopped successfully");
	simplify();
}

void NumberRecorder::valueCaught(double value) {
	//qDebug("%i gone", m_base->msecsTo(&QDateTime::currentDateTime()));
	m_evcount++;
	
	// This whole stack/heap thing is getting to me
	// Couldn't we have done it in Java instead :)
	QDateTime now = QDateTime::currentDateTime();
	
	int delta = m_base->msecsTo(&now);
	
	if (m_times.empty()) {
		m_times.append(delta);
		m_values.append(value);
	} else {
		int last = m_times.last();
		if (delta > last) {
			m_times.append(delta);
			m_values.append(value);
		}
	}
}

void NumberRecorder::writeToScript(Recorder* rec) {
	if (m_values.empty()) {
		return;
	}
	rec->beginInterpolate(m_group, m_name, m_interp);

	QList<int>::const_iterator tit;
	QList<double>::const_iterator vit;

	vit = m_values.begin();
	for (tit = m_times.begin(); tit != m_times.end(); tit++) {
		rec->addInterPoint(*tit, *vit);
		vit++;
	}
	rec->endInterpolate();
}

#define TOLERANCE 0.05

/**
 * This function attempts to find sets of points which are almost colinear
 * in the signal recorded. It uses a pretty naive algorithm for doing this.
 */

void NumberRecorder::simplify() {
	if (m_times.count() < 3) {
		return;
	}
	for (int i = 0; i < m_times.count() - 2; i++) {
//		qDebug("Count: %i", m_times.count());
		int end = findFurthest(i);
//		qDebug("End: %i->%i", i, end);
		if (i > 200) { exit(0); }
		if (end > i + 1) {
			QList<int>::iterator tst = m_times.begin();
			tst++;
			QList<double>::iterator vst = m_values.begin();
			vst++;
//			qDebug("Erasing %i at %i", ((end - i) - 1), i);
			for (int d = 0; d < ((end - i) - 1); d++) {
				tst = m_times.erase(tst);
				vst = m_values.erase(vst);
			}
		}
	}
}
	
int NumberRecorder::findFurthest(int start) {
	int max = m_times.count() - 1;
	if (start + 1 == max) {
		return max;
	}
	for (int i = start+2; i <= max; i++) {
		if (!tryLineFit(start, i)) {
			return i - 1;
		}
	}
	return max;
}

bool NumberRecorder::tryLineFit(int start, int end) {
	double m = (m_values[end]-m_values[start])/
		(m_times[end]-m_times[start]);
	double c = m_values[start] - (m*m_times[start]);
//	qDebug("Linear fit: %f, %f", (float)m, (float)c);
//	qDebug("From: %i, %f", m_times[start], (float)m_values[start]);
//	qDebug("To: %i, %f", m_times[end], (float)m_values[end]);
	for (int i = start + 1; i < end; i++) {
		double y = (m * m_times[i]) + c;
		double diff = y - m_values[i];
		if (diff < 0.0) { diff = -diff; }
		//qDebug("Diff here: %f", (float)diff);
		if (diff > TOLERANCE) { return false; }
	}

	return true;
}
