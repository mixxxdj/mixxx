#ifndef SCRIPT_SIGNALRECORDER_H
#define SCRIPT_SIGNALRECORDER_H

#include "sdatetime.h"
#include "luarecorder.h"
#include "../controlobjectthreadmain.h"

#include <qvaluelist.h>
#include <qobject.h>
#include <qtimer.h>

class SignalRecorder : public QObject {
	Q_OBJECT
	
	public:
		SignalRecorder(const char* group, const char* name);
		~SignalRecorder();
	
		void startRecord(SDateTime *base);
		void stopRecord();
	
		void writeToScript(LuaRecorder *rec);
		void reset();
	public slots:
		void valueCaught(double);
		
	private:
		const char* m_group;
		const char* m_name;
		QValueList<int>* m_times;
		QValueList<double>* m_values;
		SDateTime *m_base;
		int m_evcount;

		ControlObjectThreadMain* m_p;
};


#endif
