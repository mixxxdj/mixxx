#include "scriptcontrolevent.h"

ScriptControlEvent::ScriptControlEvent(QDateTime time, int process, int tag) {
	m_time = time;
	m_process = process;
	m_tag = tag;
}

ScriptControlEvent::~ScriptControlEvent() {
}

const QDateTime* ScriptControlEvent::getTime() {
	return &m_time;
}

void ScriptControlEvent::setProcess(int process) {
	m_process = process;
}

int ScriptControlEvent::getProcess() {
	return m_process;
}

void ScriptControlEvent::setTag(int tag) {
	m_tag = tag;
}

int ScriptControlEvent::getTag() {
	return m_tag;
}
