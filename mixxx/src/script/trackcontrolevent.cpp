#include "trackcontrolevent.h"

TrackControlEvent::TrackControlEvent(ScriptEngine* parent, int channel,
		QString path, QDateTime time, int process, int tag) \
		: ScriptControlEvent(time, process, tag) {
	m_parent = parent;
	m_channel = channel;
	m_path = path;
}

TrackControlEvent::~TrackControlEvent() {
}

void TrackControlEvent::execute() {
	//qDebug(m_path);
	m_parent->playTrack(m_channel, m_path);
}
