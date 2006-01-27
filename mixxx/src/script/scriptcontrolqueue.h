#ifndef SCRIPT_SCRIPTCONTROLQUEUE_H
#define SCRIPT_SCRIPTCONTROLQUEUE_H

#include <qptrlist.h>
#include <qdatetime.h>
#include <qtimer.h>
#include <qstring.h>
#include "scriptcontrolevent.h"

class ScriptEngine;

class ScriptControlQueue : public QObject {
	Q_OBJECT
	
	public:
		ScriptControlQueue(ScriptEngine* parent);
		~ScriptControlQueue();
		void schedule(ScriptControlEvent *event);
		void schedule(int channel, QString path, QDateTime base, int offset);
		void schedule(const char* group, const char* name, double value, const QDateTime *base, int offset);
		void interpolate(const char* group, const char* name, const QDateTime *base, int time1, double val1, int time2, double val2, bool addLast = TRUE, int minres = 50);
	public slots:
		void timerCallback();
		
	protected:
		void setupCallbacks();
		QDateTime getWhen(const QDateTime* base, int offset);

	    	QPtrList<ScriptControlEvent> m_q;
		QTimer m_timer;

		ScriptEngine* m_parent;
};

#endif
