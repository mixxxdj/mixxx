#include "numbercontrolevent.h"
#include "../controlobject.h"

NumberControlEvent::NumberControlEvent(const char* group, const char* name, \
		double value, QDateTime time, int process, int tag)\
		: ScriptControlEvent(time, process, tag) {
	m_obj = ControlObject::getControl(ConfigKey(group, name));
	m_value = value;
}

NumberControlEvent::NumberControlEvent(ControlObject *obj, double value, QDateTime time, int process, int tag) : ScriptControlEvent(time, process, tag) {
	m_obj = obj;
	m_value = value;
}

NumberControlEvent::~NumberControlEvent() {
}

void NumberControlEvent::execute() {
	m_obj->queueFromThread(m_value);
}

double NumberControlEvent::getValue() {
	return m_value;
}
