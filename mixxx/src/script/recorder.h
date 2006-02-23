#ifndef SCRIPT_RECORDER_H
#define SCRIPT_RECORDER_H

#include <qstring.h>

#include "interp.h"

class Recorder {
	public:
		Recorder(QString *macro);
		virtual ~Recorder();

		virtual void beginInterpolate(const char* group, const char* name, int interp = INTERP_LINEAR) = 0;
		virtual void addInterPoint(int time, double value) = 0;
		virtual void endInterpolate() = 0;

		virtual void playChannel(int channel, int time, QString path) = 0;
		
		QString* getMacro();
	protected:
		QString* m_macro;
};

#endif
