#ifndef SCRIPT_SCRIPTENGINE_H
#define SCRIPT_SCRIPTENGINE_H

#include "../mixxx.h"
#include "luainterface.h"
#include "macro.h"
#include "scriptstudio.h"
#include <qptrlist.h>

class ScriptStudio;

class ScriptEngine {
	public:
		ScriptEngine(MixxxApp* parent);
		~ScriptEngine();
		
		void executeScript(const char* script);
		void addMacro(Macro* macro);
		void newMacro();
		void deleteMacro(Macro* macro);
		int macroCount();
		Macro* getMacro(int index);

		void saveMacros();
		ScriptStudio* getStudio();
	private:
		QFile* getMacroFile();
	
		ScriptStudio* m_studio;
		LuaInterface *m_lua;
		QPtrList<Macro>* m_macros;

		void loadMacros();
};

#endif
