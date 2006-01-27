#include "luainterface.h"
#include "scriptengine.h"
#include <tolua.h>
#include <string.h>

#include "../controlobject.h"
#include "../controlpotmeter.h"
#include <qapplication.h>
#include <qdatetime.h>

#include "interp.h"

extern int tolua_mixxx_open(lua_State*);

LuaInterface::LuaInterface(ScriptControlQueue* q) {
	m_q = q;
	
	qDebug("Creating Lua interpreter...");
	m_L = lua_open();
	
	luaopen_base(m_L);
	luaopen_table(m_L);
	luaopen_io(m_L);
	luaopen_string(m_L);
	luaopen_math(m_L);
	
	tolua_mixxx_open(m_L);
	
	
	lua_pushstring(m_L, "mixxx");
	tolua_pushusertype(m_L, this, "LuaInterface");
	lua_settable(m_L, LUA_GLOBALSINDEX);
	
	qDebug("Lua interpreter initialised");
	
	/*char buf[] = {"mixxx:test()"};
	int error = luaL_loadbuffer(L, buf, strlen(buf), "line") ||
                lua_pcall(L, 0, 0, 0);
	
	if (error) {
          fprintf(stderr, "%s", lua_tostring(L, -1));
          lua_pop(L, 1);
        }*/
	
	m_group = 0;
	m_name = 0;
}

LuaInterface::~LuaInterface() {
}

void LuaInterface::executeScript(const char *script) {

	int error = luaL_loadbuffer(m_L, script, strlen(script), "line") ||
                lua_pcall(m_L, 0, 0, 0);
	
	if (error) {
		fprintf(stderr, "%s", lua_tostring(m_L, -1));
		lua_pop(m_L, 1);
	}
}

void LuaInterface::test() {
	QDateTime base = QDateTime::currentDateTime();
	m_q->interpolate("[Master]", "crossfader", &base, 2000, -1.0, 4000, 1.0);
}

void LuaInterface::stop(int channel) {
	if (channel == 1) {
		ControlObject *p = ControlObject::getControl(ConfigKey("[Channel1]","play"));
                p->queueFromThread(0.);
	} else if (channel == 2) {
		ControlObject *p = ControlObject::getControl(ConfigKey("[Channel2]","play"));
                p->queueFromThread(0.);
	} else {
		qDebug("LuaInterface: No such channel %i to stop", channel);
	}
}

void LuaInterface::play(int channel) {
	if (channel == 1) {
		ControlObject *p = ControlObject::getControl(ConfigKey("[Channel1]","play"));
                p->queueFromThread(1.);
	} else if (channel == 2) {
		ControlObject *p = ControlObject::getControl(ConfigKey("[Channel2]","play"));
                p->queueFromThread(1.);
	} else {
		qDebug("LuaInterface: No such channel %i to play", channel);
	}
}

double LuaInterface::getFader() {
	ControlObject *pot = ControlObject::getControl(ConfigKey("[Master]", "crossfader"));
	return pot->get();
}

double LuaInterface::getValue(const char* group, const char* name) {
	ControlObject *pot = ControlObject::getControl(ConfigKey(group, name));
	if (pot == NULL) {
		qDebug("Unknown control %s:%s", group, name);
		return 0.0;
	}
	return pot->get();
}

void LuaInterface::setFader(double fade) {
	ControlObject *pot = ControlObject::getControl(ConfigKey("[Master]", "crossfader"));
	pot->queueFromThread(fade);
}

void LuaInterface::startFadeCrossfader() {
	startFade("[Master]", "crossfader");
}

void LuaInterface::startList(const char* group, const char* name) {
	startFade(group, name, INTERP_NONE);
}

void LuaInterface::startFade(const char* group, const char* name) {
	startFade(group, name, INTERP_LINEAR);
}

void LuaInterface::startFade(const char* group, const char* name, int interp) {
	if (m_group != 0) { 
		qDebug("startFade before endFade");
	}
	m_group = group;
	m_name = name;
	m_times = new QValueList<int>();
	m_values = new QValueList<double>();
	m_time = QDateTime::currentDateTime();

	m_interp = interp;
}

void LuaInterface::fadePoint(int time, double value) {
	m_times->append(time);
	m_values->append(value);
}

void LuaInterface::point(int time, double value) {
	fadePoint(time, value);
}

void LuaInterface::endList() { endFade(); }

void LuaInterface::endFade() {
	
	QValueListConstIterator<int> ti = m_times->constBegin();
	QValueListConstIterator<double> vi = m_values->constBegin();
	
	int last = *ti;
	double value = *vi;
	ti++; vi++;

	if (m_interp == INTERP_NONE) {
		qDebug("Hello %i", last);
		m_q->schedule(m_group, m_name, value, &m_time, last);
	}
	
	while(ti != m_times->end()) {
		if (m_interp == INTERP_LINEAR) {
			m_q->interpolate(m_group, m_name, &m_time, 
					last, value, *ti, *vi, TRUE);
		} else {
			m_q->schedule(m_group, m_name, *vi, &m_time, *ti);
		}
			
		last = *ti;
		value = *vi;
		ti++; vi++;
	}
	
	delete m_times;
	delete m_values;

	m_group = 0;
	m_name = 0;
}

void LuaInterface::playChannel1(int time, char* path) {
	m_q->schedule(1, path, QDateTime::currentDateTime(), time);
}

void LuaInterface::playChannel2(int time, char* path) {
	m_q->schedule(2, path, QDateTime::currentDateTime(), time);
}
