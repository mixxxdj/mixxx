#ifndef SCRIPT_LUAINTERFACE_H
#define SCRIPT_LUAINTERFACE_H

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

#include "../scriptcontrolqueue.h"
#include "qvaluelist.h"
#include "qdatetime.h"
#include "../playinterface.h"

class LuaInterface {
	public:
		LuaInterface(PlayInterface *pi);
		~LuaInterface();
		
		void executeScript(const char *script, int process);
	private:
		PlayInterface* m_pi;
		lua_State* m_L;
};

#endif
