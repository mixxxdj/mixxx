#ifndef SCRIPT_SCRIPTENGINE_H
#define SCRIPT_SCRIPTENGINE_H

#include "../mixxx.h"
#include "luainterface.h"

class ScriptEngine {
	public:
		ScriptEngine(MixxxApp* parent);
		~ScriptEngine();
		
		void executeScript(const char* script);
	private:
		LuaInterface *m_lua;
};

#endif
