#ifndef SCRIPT_SCRIPTCONTROLEVENT_H
#define SCRIPT_SCRIPTCONTROLEVENT_H

#include <qdatetime.h>

class ScriptControlEvent {
	public:
		ScriptControlEvent(QDateTime time, int process, int tag = -1);
		virtual ~ScriptControlEvent();
		virtual void execute() = 0;
		const QDateTime* getTime();

		int getProcess();
		void setProcess(int);
		int getTag();
		void setTag(int);
	protected:
		QDateTime m_time;
		int m_process;
		int m_tag;
};

#endif
