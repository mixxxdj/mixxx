#ifndef SCRIPT_SCRIPTCONTROLQUEUE_H
#define SCRIPT_SCRIPTCONTROLQUEUE_H

#include <qptrlist.h>
#include <qdatetime.h>
#include <qtimer.h>
#include "scriptcontrolevent.h"

class ScriptControlQueue : public QObject {
	Q_OBJECT
	
	public:
		ScriptControlQueue();
		~ScriptControlQueue();
		void schedule(ScriptControlEvent *event);
		void schedule(const char* group, const char* name, double value, const QDateTime *base, int offset);
		void interpolate(const char* group, const char* name, const QDateTime *base, int time1, double val1, int time2, double val2, bool addLast = TRUE, int minres = 50);
	public slots:
		void timerCallback();
		
	protected:
		void setupCallbacks();
	
	    	QPtrList<ScriptControlEvent> m_q;
		QTimer m_timer;
};

#endif
