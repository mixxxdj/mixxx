#ifndef SCRIPT_TRACKRECORDER_H
#define SCRIPT_TRACKRECORDER_H

#include "signalrecorder.h"
#include "recorder.h"
#include "../track.h"

#include <qvaluevector.h>

class TrackRecorder : public SignalRecorder {
	Q_OBJECT
	
	public:
		TrackRecorder(Track* track, int channel);

		virtual void startRecord(SDateTime* base);
		virtual void stopRecord();

		virtual void writeToScript(Recorder* rec);
		virtual void reset();
	public slots:
		void logTrack(TrackInfoObject* track);
	protected:
		SDateTime* m_base;

		QValueVector<int> m_times;
		QValueVector<QString> m_paths;

		Track* m_track;
		int m_channel;
};
#endif
