#include "scriptcontrolevent.h"
#include "../controlobject.h"

ScriptControlEvent::ScriptControlEvent(const char* group, const char* name, double value, const QDateTime *time) {
	m_obj = ControlObject::getControl(ConfigKey(group, name));
	m_value = value;
	m_time = time;
}

ScriptControlEvent::ScriptControlEvent(ControlObject *obj, double value, const QDateTime *time) {
	m_obj = obj;
	m_value = value;
	m_time = time;
}

ScriptControlEvent::~ScriptControlEvent() {
}

void ScriptControlEvent::execute() {
	m_obj->queueFromThread(m_value);
}

const QDateTime* ScriptControlEvent::getTime() {
	return m_time;
}

double ScriptControlEvent::getValue() {
	return m_value;
}
