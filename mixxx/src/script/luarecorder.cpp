#include "luarecorder.h"

LuaRecorder::LuaRecorder(QString* macro) {
	m_macro = macro;
}

LuaRecorder::~LuaRecorder() {
}

void LuaRecorder::beginInterpolate(const char* group, const char* name) {
	m_macro->append("mixxx:startFade(\"");
	m_macro->append(group);
	m_macro->append("\", \"");
	m_macro->append(name);
	m_macro->append("\"); ");
}

void LuaRecorder::addInterPoint(int time, double value) {
	m_macro->append("mixxx:fadePoint(");
	QString tstr = QString::number(time);
	QString vstr = QString::number(value, 'f');
	m_macro->append(tstr);
	m_macro->append(", ");
	m_macro->append(vstr);
	m_macro->append("); ");
}

void LuaRecorder::endInterpolate() {
	m_macro->append("mixxx:endFade(); ");
}

QString* LuaRecorder::getMacro() {
	return m_macro;
}
