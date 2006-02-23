#include "macro.h"

#include <qobject.h>

Macro::Macro(int lang, QString name, QString script) {
	m_lang = lang;
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

void Macro::setLang(int lang) {
	m_lang = lang;
}

int Macro::getLang() {
	return m_lang;
}

QString Macro::getLangName() {
	if (m_lang == LANG_PYTHON) {
		return "Python";
	} else if (m_lang == LANG_LUA) {
		return "Lua";
	} else {
		return "Unknown";
	}
}
