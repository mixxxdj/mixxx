#include "macro.h"

#include <qobject.h>

Macro::Macro(QString name, QString script) {
	m_name = name;
	m_script = script;
}

Macro::~Macro() {
}

void Macro::setName(QString name) {
	m_name = name;
}

QString Macro::getName() {
	return m_name;
}

void Macro::setScript(QString script) {
	m_script = script;
}

QString Macro::getScript() {
	return m_script;
}
