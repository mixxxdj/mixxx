#ifndef SCRIPT_SCRIPTENGINE_H
#define SCRIPT_SCRIPTENGINE_H

#include "../mixxx.h"
#include "../track.h"
#include "playinterface.h"
#include "qtscriptinterface.h"

#ifdef __LUA__
	#include "lua/luainterface.h"
#endif

#include "macro.h"
#include "scriptstudio.h"
#include <QList>
#include <qstring.h>

class ScriptStudio;
class ScriptRecorder;

class ScriptEngine {
	public:
		ScriptEngine(MixxxApp* parent, Track* track);
		~ScriptEngine();
	
		void executeMacro(Macro* macro);
		void executeScript(const char* script);
		void addMacro(Macro* macro);
		void newMacro(int lang);
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

		PlayInterface *m_pi;
		QtScriptInterface* m_qti;
#ifdef __LUA__
		LuaInterface *m_lua;
#endif
		QList<Macro*>* m_macros;

		void loadMacros();

		int m_pcount;
};

#endif
