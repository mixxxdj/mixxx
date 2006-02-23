#ifndef SCRIPT_SCRIPTRECORDER_H
#define SCRIPT_SCRIPTRECORDER_H

#include "signalrecorder.h"
#include "macro.h"
#include "../track.h"

#include <qstring.h>
#include <qptrlist.h>

class ScriptRecorder {
	public:
		ScriptRecorder(Track* track);
		~ScriptRecorder();

		void startRecord();
		void stopRecord();
		Macro* getMacro();
		void reset();
		
	private:
		QPtrList<SignalRecorder> *m_all;
		SignalRecorder* m_crossfader;
		Track* m_track;
		
		void install(const char* group, const char* name);
		void installTrack(int channel);
		void installRaw(const char* group, const char* name);
};

#endif
