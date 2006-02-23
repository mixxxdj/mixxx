#ifndef SCRIPT_TRACKCONTROLEVENT_H
#define SCRIPT_TRACKCONTROLEVENT_H

#include "scriptcontrolevent.h"
#include "scriptengine.h"

class TrackControlEvent : public ScriptControlEvent {
	public:
		TrackControlEvent(ScriptEngine* parent, int channel, \
				QString path, QDateTime time, int process, \
				int tag = -1);
		virtual ~TrackControlEvent();
		virtual void execute();
	protected:
		ScriptEngine* m_parent;
		int m_channel;
		QString m_path;
};
#endif
