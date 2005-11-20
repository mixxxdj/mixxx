#ifndef SCRIPT_LUARECORDER_H
#define SCRIPT_LUARECORDER_H

#include <qstring.h>

class LuaRecorder {
	public:
		LuaRecorder(QString *macro);
		~LuaRecorder();

		void beginInterpolate(const char* group, const char* name);
		void addInterPoint(int time, double value);
		void endInterpolate();

		QString* getMacro();
	private:
		QString* m_macro;
};

#endif
