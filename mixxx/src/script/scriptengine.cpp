#include "luainterface.h"
#include "scriptengine.h"
#include "scripttest.h"
#include "scriptcontrolqueue.h"
#include "scriptrecorder.h"

ScriptEngine::ScriptEngine(MixxxApp* parent) {
	ScriptControlQueue* q = new ScriptControlQueue();
	m_lua = new LuaInterface(q); 

	ScriptTest* stest = new ScriptTest(this);
	ScriptRecorder* bobby = new ScriptRecorder();
}

ScriptEngine::~ScriptEngine() {
}

void ScriptEngine::executeScript(const char* script) {
	// Don't execute a null script
	if (script == 0x0) {
		return;
	}
	// For now just call lua, but this layer is here in anticipation of not
	// necessarily wanting to use it
	m_lua->executeScript(script);
}
