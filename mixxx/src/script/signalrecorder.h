#ifndef SCRIPT_SIGNALRECORDER_H
#define SCRIPT_SIGNALRECORDER_H

#include <qobject.h>

#include "sdatetime.h"
#include "recorder.h"

class SignalRecorder : public QObject {
	public:
		SignalRecorder();
		virtual ~SignalRecorder();

		// TODO: This should be SDateTime not SDateTime*
		// will fix a memory leak (20 bytes per record or something?)
                virtual void startRecord(SDateTime *base) = 0;
                virtual void stopRecord() = 0;

                virtual void writeToScript(Recorder *rec) = 0;
                virtual void reset() = 0;
};
#endif
