#include "recorder.h"

Recorder::Recorder(QString* macro) {
	m_macro = macro;
}

Recorder::~Recorder() {
}

QString* Recorder::getMacro() {
	return m_macro;
}
