#ifndef SCRIPT_MACRO_H
#define SCRIPT_MACRO_H

#include <qstring.h>

class Macro {
	public:
		Macro(int lang, QString name = "", QString script = "");
		~Macro();
		void setName(QString name);
		QString getName();
		void setScript(QString script);
		QString getScript();
		void setLang(int lang);
		int getLang();

		QString getLangName();
		
		static const int LANG_PYTHON = 1;
		static const int LANG_LUA = 2;
		static const int LANG_QTSCRIPT = 3;
	private:
		QString m_name;
		QString m_script;
		int m_lang;
};

#endif
