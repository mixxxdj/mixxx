#include "luainterface.h"
#include "../scriptengine.h"
#include <tolua.h>
#include <string.h>

#include "../controlobject.h"
#include "../controlpotmeter.h"
#include <qapplication.h>
#include <qdatetime.h>

#include "../interp.h"

extern int tolua_mixxx_open(lua_State*);

LuaInterface::LuaInterface(PlayInterface* pi) {
	m_pi = pi;
	
	qDebug("Creating Lua interpreter...");
	m_L = lua_open();
	
	luaopen_base(m_L);
	luaopen_table(m_L);
	luaopen_io(m_L);
	luaopen_string(m_L);
	luaopen_math(m_L);
	
	tolua_mixxx_open(m_L);
	
	lua_pushstring(m_L, "mixxx");
	tolua_pushusertype(m_L, m_pi, "PlayInterface");
	lua_settable(m_L, LUA_GLOBALSINDEX);
	
	qDebug("Lua interpreter initialised");
	
	/*char buf[] = {"mixxx:test()"};
	int error = luaL_loadbuffer(L, buf, strlen(buf), "line") ||
                lua_pcall(L, 0, 0, 0);
	
	if (error) {
          fprintf(stderr, "%s", lua_tostring(L, -1));
          lua_pop(L, 1);
        }*/
}

LuaInterface::~LuaInterface() {
}

void LuaInterface::executeScript(const char *script, int process) {

	m_pi->setProcess(process);
	int error = luaL_loadbuffer(m_L, script, strlen(script), "line") ||
                lua_pcall(m_L, 0, 0, 0);
	m_pi->clearProcess();
	
	if (error) {
		fprintf(stderr, "%s", lua_tostring(m_L, -1));
		lua_pop(m_L, 1);
	}
}
