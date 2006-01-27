#include "scriptcontrolevent.h"

ScriptControlEvent::ScriptControlEvent(QDateTime time) {
	m_time = time;
}

ScriptControlEvent::~ScriptControlEvent() {
}

const QDateTime* ScriptControlEvent::getTime() {
	return &m_time;
}
