#include "signalrecorder.h"
#include "scriptrecorder.h"

ScriptRecorder::ScriptRecorder() {

	m_all = new QPtrList<SignalRecorder>();
	install("[Master]", "crossfader");
	install("[Master]", "balance");
	install("[Master]", "volume");

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

}

void ScriptRecorder::install(const char* group, const char* name) {
	m_all->append(new SignalRecorder(group, name));	
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

QString* ScriptRecorder::getMacro() {
	QString *macro = new QString();
	LuaRecorder *rec = new LuaRecorder(macro);

        SignalRecorder *ptr;
        for (ptr = m_all->first(); ptr; ptr = m_all->next()) {
                ptr->writeToScript(rec);
        }

	delete rec;
	return macro;
}

void ScriptRecorder::reset() {
	SignalRecorder *ptr;
	for (ptr = m_all->first(); ptr; ptr = m_all->next()) {
		ptr->reset();
	}
}
