#ifndef SCRIPT_SCRIPTCONTROLEVENT_H
#define SCRIPT_SCRIPTCONTROLEVENT_H

#include <qdatetime.h>

class ScriptControlEvent {
	public:
		ScriptControlEvent(QDateTime time);
		virtual ~ScriptControlEvent();
		virtual void execute() = 0;
		const QDateTime* getTime();
	protected:
		QDateTime m_time;
};

#endif
