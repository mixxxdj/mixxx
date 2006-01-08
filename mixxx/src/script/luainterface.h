#ifndef SCRIPT_LUAINTERFACE_H
#define SCRIPT_LUAINTERFACE_H

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

#include "scriptcontrolqueue.h"
#include "qvaluelist.h"
#include "qdatetime.h"

class LuaInterface {
	public:
		LuaInterface(ScriptControlQueue* q);
		~LuaInterface();
		
		void stop(int channel);
		void play(int channel);
		void setFader(double fade);
		void test();

		double getFader();
		double getValue(const char* group, const char* name);
		
		void executeScript(const char *script);
		
		void startFadeCrossfader();
		void startList(const char* group, const char* name);
		void startFade(const char* group, const char* name);
		void startFade(const char* group, const char* name, int interp);
		void point(int time, double value);
		void fadePoint(int time, double value);
		void endFade();
		void endList();
	private:
		ScriptControlQueue* m_q;
		lua_State* m_L;
		
		QDateTime m_time;
		QValueList<int>* m_times;
		QValueList<double>* m_values;
		
		const char* m_group;
		const char* m_name;
		int m_interp;
};

#endif
