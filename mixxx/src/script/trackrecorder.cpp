#include "trackrecorder.h"

TrackRecorder::TrackRecorder(Track* track, int channel) : SignalRecorder() {
	m_times = QList<int>();
	m_paths = QList<QString>();
	m_base = 0;

	m_track = track;
	m_channel = channel;
}

void TrackRecorder::reset() {
	m_times = QList<int>();
	m_paths = QList<QString>();
	stopRecord();
}

void TrackRecorder::startRecord(SDateTime* base) {
	m_base = base;

	if (m_channel == 1) {	
		connect(m_track, SIGNAL(newTrackPlayer1(TrackInfoObject*)),
			this, SLOT(logTrack(TrackInfoObject*)));
	} else if (m_channel == 2) {
		connect(m_track, SIGNAL(newTrackPlayer2(TrackInfoObject*)),
                        this, SLOT(logTrack(TrackInfoObject*)));
	} else {
		qDebug("Unknown channel (%i) in recorder", m_channel);
	}
}

void TrackRecorder::stopRecord() {
	m_track->disconnect(this);
}

void TrackRecorder::logTrack(TrackInfoObject* track) {
	QDateTime now = QDateTime::currentDateTime();

	int delta = m_base->msecsTo(&now);

	m_times.append(delta);
	m_paths.append(track->getLocation());

}

void TrackRecorder::writeToScript(Recorder* rec) {
	if (m_times.empty()) {
		return;
	}

	QList<int>::const_iterator tit;
	QList<QString>::const_iterator pit;
	pit = m_paths.begin();
	for (tit = m_times.begin(); tit != m_times.end(); tit++) {
		rec->playChannel(m_channel , *tit, *pit);
		pit++;
	}
}
