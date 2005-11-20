#include "signalrecorder.h"
#include "../controlobjectthreadmain.h"
#include "../controlobject.h"
#include "../configobject.h"

#include <qdatetime.h>

SignalRecorder::SignalRecorder(const char* group, const char* name) :
	QObject(0, "SignalRecorder") {
	m_group = group;
	m_name = name;
	m_evcount = 0;

	m_times = new QValueList<int>();
	m_values = new QValueList<double>();
}

SignalRecorder::~SignalRecorder() {
}

void SignalRecorder::reset() {
	delete m_times;
	delete m_values;
	m_times = new QValueList<int>();
	m_values = new QValueList<double>();
	m_evcount = 0;

	stopRecord();
	
	// This broke a lot of stuff
	//delete m_p;

	// TODO: The SDateTime object which holds the start time is probably a
	// memory leak
}

void SignalRecorder::startRecord(SDateTime *base) {
	m_base = base;

        ControlObject *obj = ControlObject::getControl(ConfigKey(m_group, m_name));
        m_p = new ControlObjectThreadMain(obj);
        connect(m_p, SIGNAL(valueChanged(double)), this, SLOT(valueCaught(double)));
        //qDebug("Started recorder for %s:%s", m_group, m_name);
}

void SignalRecorder::stopRecord() {
	m_p->disconnect(this);
}

void SignalRecorder::valueCaught(double value) {
	//qDebug("%i gone", m_base->msecsTo(&QDateTime::currentDateTime()));
	m_evcount++;
	
	// This whole stack/heap thing is getting to me
	// Couldn't we have done it in Java instead :)
	QDateTime now = QDateTime::currentDateTime();
	
	int delta = m_base->msecsTo(&now);
	
	if (m_times->empty()) {
		m_times->append(delta);
		m_values->append(value);
	} else {
		int last = m_times->last();
		if (delta > last) {
			m_times->append(delta);
			m_values->append(value);
		}
	}
}

void SignalRecorder::writeToScript(LuaRecorder* rec) {
	if (m_values->empty()) {
		return;
	}
	rec->beginInterpolate(m_group, m_name);

	QValueList<int>::const_iterator tit;
	QValueList<double>::const_iterator vit;

	vit = m_values->begin();
	for (tit = m_times->begin(); tit != m_times->end(); tit++) {
		rec->addInterPoint(*tit, *vit);
		vit++;
	}
	rec->endInterpolate();
}
