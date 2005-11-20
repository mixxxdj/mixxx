#ifndef SCRIPT_SCRIPTRECORDER_H
#define SCRIPT_SCRIPTRECORDER_H

#include "luarecorder.h"
#include "signalrecorder.h"

#include <qstring.h>
#include <qptrlist.h>

class ScriptRecorder {
	public:
		ScriptRecorder();
		~ScriptRecorder();

		void startRecord();
		void stopRecord();
		QString* getMacro();
		void reset();
		
	private:
		QPtrList<SignalRecorder> *m_all;
		SignalRecorder* m_crossfader;

		void install(const char* group, const char* name);

};

#endif
