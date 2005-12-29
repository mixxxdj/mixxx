#ifndef SCRIPT_MACRO_H
#define SCRIPT_MACRO_H

#include <qstring.h>

class Macro {
	public:
		Macro(QString name = "", QString script = "");
		~Macro();
		void setName(QString name);
		QString getName();
		void setScript(QString script);
		QString getScript();
	private:
		QString m_name;
		QString m_script;
};

#endif
