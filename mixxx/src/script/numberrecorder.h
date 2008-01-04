#ifndef SCRIPT_NUMBERRECORDER_H
#define SCRIPT_NUMBERRECORDER_H

#include "sdatetime.h"
#include "recorder.h"
#include "signalrecorder.h"
#include "../controlobjectthreadmain.h"

#include <QList>
#include <qobject.h>
#include <qtimer.h>

#include "interp.h"

class NumberRecorder : public SignalRecorder {
	Q_OBJECT
	
	public:
		NumberRecorder(const char* group, const char* name, int interp = INTERP_LINEAR);
		virtual ~NumberRecorder();
	
		virtual void startRecord(SDateTime *base);
		virtual void stopRecord();
	
		virtual void writeToScript(Recorder *rec);
		virtual void reset();
	public slots:
		void valueCaught(double);
		
	private:
		void simplify();
		int findFurthest(int start);
		bool tryLineFit(int start, int len);
	
		const char* m_group;
		const char* m_name;
		QList<int> m_times;
		QList<double> m_values;
		SDateTime *m_base;
		int m_evcount;

		ControlObjectThreadMain* m_p;
		int m_interp;
};


#endif
