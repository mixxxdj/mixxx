#ifndef SCRIPT_SCRIPTENGINE_H
#define SCRIPT_SCRIPTENGINE_H

#include "../mixxx.h"
#include "../track.h"
#include "luainterface.h"
#include "macro.h"
#include "scriptstudio.h"
#include <qptrlist.h>
#include <qstring.h>

class ScriptStudio;
class ScriptRecorder;

class ScriptEngine {
	public:
		ScriptEngine(MixxxApp* parent, Track* track);
		~ScriptEngine();
		
		void executeScript(const char* script);
		void addMacro(Macro* macro);
		void newMacro();
		void deleteMacro(Macro* macro);
		int macroCount();
		Macro* getMacro(int index);

		void playTrack(int channel, QString filename);
		
		void saveMacros();
		ScriptStudio* getStudio();
		ScriptRecorder* getRecorder();
	private:
		QFile* getMacroFile();

		MixxxApp* m_parent;
		Track* m_track;
		ScriptRecorder* m_rec;
		ScriptStudio* m_studio;
		LuaInterface *m_lua;
		QPtrList<Macro>* m_macros;

		void loadMacros();
};

#endif
