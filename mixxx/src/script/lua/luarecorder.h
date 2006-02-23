#ifndef SCRIPT_LUARECORDER_H
#define SCRIPT_LUARECORDER_H

#include <qstring.h>

#include "../recorder.h"

class LuaRecorder : public Recorder {
	public:
		LuaRecorder(QString *macro);
		virtual ~LuaRecorder();

		virtual void beginInterpolate(const char* group, const char* name, int interp = INTERP_LINEAR);
		virtual void addInterPoint(int time, double value);
		virtual void endInterpolate();

		virtual void playChannel(int channel, int time, QString path);
	private:
		int m_interp;
};

#endif
