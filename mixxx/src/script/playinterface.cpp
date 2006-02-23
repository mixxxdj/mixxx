#include "playinterface.h"
#include "scriptengine.h"
#include <string.h>

#include "../controlobject.h"
#include "../controlpotmeter.h"
#include <qapplication.h>
#include <qdatetime.h>

#include "interp.h"

PlayInterface::PlayInterface(ScriptControlQueue* q) {
	m_q = q;

	m_process = -1;
	m_tag = -1;
	m_group = 0;
	m_name = 0;
}

PlayInterface::~PlayInterface() {
}

void PlayInterface::clearProcess() {
	m_process = -1;
	clearTag();
}
	
void PlayInterface::setProcess(int process) {
	if (m_process != -1) {
		qDebug("This process stuff won't work multithreaded yet...");
	}
	m_process = process;
}

void PlayInterface::setTag(int tag) {
	m_tag = tag;
}

void PlayInterface::clearTag() {
	m_tag = -1;
}

void PlayInterface::kill() {
	m_q->killProcess(m_process);
}

void PlayInterface::killTag(int tag) {
	m_q->killTag(m_process, tag);
}
	
void PlayInterface::test() {
	QDateTime base = QDateTime::currentDateTime();
	m_q->interpolate("[Master]", "crossfader", &base, 2000, -1.0, 4000, \
			1.0, m_process, m_tag);
}

void PlayInterface::stop(int channel) {
	if (channel == 1) {
		ControlObject *p = ControlObject::getControl(ConfigKey("[Channel1]","play"));
                p->queueFromThread(0.);
	} else if (channel == 2) {
		ControlObject *p = ControlObject::getControl(ConfigKey("[Channel2]","play"));
                p->queueFromThread(0.);
	} else {
		qDebug("PlayInterface: No such channel %i to stop", channel);
	}
}

void PlayInterface::play(int channel) {
	if (channel == 1) {
		ControlObject *p = ControlObject::getControl(ConfigKey("[Channel1]","play"));
                p->queueFromThread(1.);
	} else if (channel == 2) {
		ControlObject *p = ControlObject::getControl(ConfigKey("[Channel2]","play"));
                p->queueFromThread(1.);
	} else {
		qDebug("PlayInterface: No such channel %i to play", channel);
	}
}

double PlayInterface::getFader() {
	ControlObject *pot = ControlObject::getControl(ConfigKey("[Master]", "crossfader"));
	return pot->get();
}

double PlayInterface::getValue(const char* group, const char* name) {
	ControlObject *pot = ControlObject::getControl(ConfigKey(group, name));
	if (pot == NULL) {
		qDebug("Unknown control %s:%s", group, name);
		return 0.0;
	}
	return pot->get();
}

void PlayInterface::setFader(double fade) {
	qDebug("Set Fader %f", (float)fade);
	ControlObject *pot = ControlObject::getControl(ConfigKey("[Master]", "crossfader"));
	pot->queueFromThread(fade);
}

void PlayInterface::startFadeCrossfader() {
	startFade("[Master]", "crossfader");
}

void PlayInterface::startList(const char* group, const char* name) {
	doStartFade(group, name, INTERP_NONE);
}

void PlayInterface::startFade(const char* group, const char* name) {
	doStartFade(group, name, INTERP_LINEAR);
}

void PlayInterface::doStartFade(const char* group, const char* name, \
		int interp) {
	if (m_group != 0) { 
		qDebug("startFade before endFade");
	}
	m_group = group;
	m_name = name;
	m_times = new QValueList<int>();
	m_values = new QValueList<double>();
	m_time = QDateTime::currentDateTime();

	m_interp = interp;
}

void PlayInterface::fadePoint(int time, double value) {
	m_times->append(time);
	m_values->append(value);
}

void PlayInterface::point(int time, double value) {
	fadePoint(time, value);
}

void PlayInterface::endList() { endFade(); }

void PlayInterface::endFade() {
	
	QValueListConstIterator<int> ti = m_times->constBegin();
	QValueListConstIterator<double> vi = m_values->constBegin();
	
	int last = *ti;
	double value = *vi;
	ti++; vi++;

	if (m_interp == INTERP_NONE) {
		m_q->schedule(m_group, m_name, value, &m_time, last, \
				m_process, m_tag);
	}
	
	while(ti != m_times->end()) {
		if (m_interp == INTERP_LINEAR) {
			m_q->interpolate(m_group, m_name, &m_time, 
					last, value, *ti, *vi, m_process, \
					m_tag, TRUE);

		} else {
			m_q->schedule(m_group, m_name, *vi, &m_time, *ti, \
					m_process, m_tag);
		}
			
		last = *ti;
		value = *vi;
		ti++; vi++;
	}
	
	delete m_times;
	delete m_values;

	m_group = 0;
	m_name = 0;
}

void PlayInterface::playChannel1(int time, char* path) {
	m_q->schedule(1, path, QDateTime::currentDateTime(), time, m_process, \
			m_tag);
}

void PlayInterface::playChannel2(int time, char* path) {
	m_q->schedule(2, path, QDateTime::currentDateTime(), time, m_process, \
			m_tag);
}
