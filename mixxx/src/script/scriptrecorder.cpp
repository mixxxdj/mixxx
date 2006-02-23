#include "signalrecorder.h"
#include "scriptrecorder.h"
#include "numberrecorder.h"
#include "trackrecorder.h"
#include "interp.h"

#ifdef __LUA__
#include "lua/luarecorder.h"
#endif

ScriptRecorder::ScriptRecorder(Track* track) {
	m_track = track;
	
	m_all = new QPtrList<SignalRecorder>();
	install("[Master]", "crossfader");
	install("[Master]", "balance");
	install("[Master]", "volume");
	installRaw("[Master]", "headVolume");
	installRaw("[Master]", "headMix");
	
	install("[Channel1]", "volume");
	install("[Channel2]", "volume");
	install("[Channel1]", "pregain");
	install("[Channel2]", "pregain");
	install("[Channel1]", "rate");
	install("[Channel2]", "rate");
	install("[Channel1]", "filterLow");
	install("[Channel1]", "filterMid");
	install("[Channel1]", "filterHigh");
        install("[Channel2]", "filterLow");
        install("[Channel2]", "filterMid");
        install("[Channel2]", "filterHigh");

	installTrack(1);
	installTrack(2);

	installRaw("[Channel1]", "play");
	installRaw("[Channel2]", "play");
	installRaw("[Channel1]", "reverse");
	installRaw("[Channel2]", "reverse");
	installRaw("[Channel1]", "fwd");
	installRaw("[Channel2]", "fwd");
	installRaw("[Channel1]", "back");
	installRaw("[Channel2]", "back");

	install("[Flanger]", "lfoDepth");
	install("[Flanger]", "lfoDelay");
	install("[Flanger]", "lfoPeriod");

	installRaw("[Channel1]", "flanger");
	installRaw("[Channel2]", "flanger");

	installRaw("[Channel1]", "loop");
	installRaw("[Channel2]", "loop");

	installRaw("[Channel1]", "cue_set");
	installRaw("[Channel2]", "cue_set");
	installRaw("[Channel1]", "cue_preview");
	installRaw("[Channel2]", "cue_preview");
	
}

void ScriptRecorder::install(const char* group, const char* name) {
	m_all->append(new NumberRecorder(group, name));	
}

void ScriptRecorder::installRaw(const char* group, const char* name) {
	m_all->append(new NumberRecorder(group, name, INTERP_NONE));
}

void ScriptRecorder::installTrack(int channel) {
	m_all->append(new TrackRecorder(m_track, channel));
}

ScriptRecorder::~ScriptRecorder() {
}

void ScriptRecorder::startRecord() {
	SDateTime* start = SDateTime::now();

	SignalRecorder *ptr;
	for (ptr = m_all->first(); ptr; ptr = m_all->next()) {
		ptr->startRecord(start);
	}
}

void ScriptRecorder::stopRecord() {
	SignalRecorder *ptr;
        for (ptr = m_all->first(); ptr; ptr = m_all->next()) {
                ptr->stopRecord();
        }
}

Macro* ScriptRecorder::getMacro() {
#ifdef __LUA__
	Macro* rmacro = new Macro(Macro::LANG_LUA, "Recorded Macro");
	QString *macro = new QString();
	LuaRecorder *rec = new LuaRecorder(macro);
#else
	qDebug("Lua support required for recording macros");
	return NULL;
#endif

#ifdef __LUA__
        SignalRecorder *ptr;
        for (ptr = m_all->first(); ptr; ptr = m_all->next()) {
                ptr->writeToScript(rec);
        }

	delete rec;
	rmacro->setScript(*macro);
	delete macro;
	return rmacro;
#endif
}

void ScriptRecorder::reset() {
	SignalRecorder *ptr;
	for (ptr = m_all->first(); ptr; ptr = m_all->next()) {
		ptr->reset();
	}
}
