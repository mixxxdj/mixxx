#ifndef SCRIPT_LUARECORDER_H
#define SCRIPT_LUARECORDER_H

#include <qstring.h>

#include "interp.h"

class LuaRecorder {
	public:
		LuaRecorder(QString *macro);
		~LuaRecorder();

		void beginInterpolate(const char* group, const char* name, int interp = INTERP_LINEAR);
		void addInterPoint(int time, double value);
		void endInterpolate();

		void playChannel(int channel, int time, QString path);
		
		QString* getMacro();
	private:
		QString* m_macro;
		int m_interp;
};

#endif
