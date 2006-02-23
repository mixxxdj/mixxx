#include "luarecorder.h"

LuaRecorder::LuaRecorder(QString* macro) : Recorder(macro) {
}

LuaRecorder::~LuaRecorder() {
}

void LuaRecorder::beginInterpolate(const char* group, const char* name
		, int interp) {
	m_interp = interp;
	if (m_interp == INTERP_NONE) {
		m_macro->append("mixxx:startList(\"");
	} else if (m_interp == INTERP_LINEAR) {	
		m_macro->append("mixxx:startFade(\"");
	} else {
		qDebug("Unknown interpolation type in LuaRecorder");
	}
	m_macro->append(group);
	m_macro->append("\", \"");
	m_macro->append(name);
	m_macro->append("\");\n");
}

void LuaRecorder::addInterPoint(int time, double value) {
	m_macro->append("\tmixxx:point(");
	QString tstr = QString::number(time);
	QString vstr = QString::number(value, 'f');
	m_macro->append(tstr);
	m_macro->append(", ");
	m_macro->append(vstr);
	m_macro->append(");\n");
}

void LuaRecorder::endInterpolate() {
	if (m_interp == INTERP_NONE) {
		m_macro->append("mixxx:endList();\n");
	} else if (m_interp == INTERP_LINEAR) {
		m_macro->append("mixxx:endFade();\n");
	} else {
		qDebug("Unkown interpolation type in LuaRecorder (end)");
	}
}

void LuaRecorder::playChannel(int channel, int time, QString path) {
	m_macro->append("mixxx:playChannel");
	m_macro->append(QString::number(channel));
	m_macro->append("(");
	m_macro->append(QString::number(time));
	m_macro->append(", \"");
	m_macro->append(path);
	m_macro->append("\");\n");
}
