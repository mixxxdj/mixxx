#ifndef QTSCRIPTINTERFACE_H
#define QTSCRIPTINTERFACE_H

#include "playinterface.h"

#include <QtScript>

class QtScriptInterface : public QObject {

	Q_OBJECT

public:
	QtScriptInterface(PlayInterface *pi);

// Functions that point back to playinterface
// Not implementing the old fader ones for now since they're bad
public slots:
	void test();
	
	void stop(int channel);
	void play(int channel);
//	void setFader(double fade);

	void setTag(int tag);
	void clearTag();
	
	void kill();
	void killTag(int tag);
		
//	double getFader();
	double getValue(const char* group, const char* name);
		
//	void startFadeCrossfader();
	void startList(const char* group, const char* name);
	void startFade(const char* group, const char* name);
	void point(int time, double value);
//	void fadePoint(int time, double value);
	void endFade();
	void endList();

	void playChannel1(int time, char* path);
	void playChannel2(int time, char* path);

private:
	PlayInterface* m_pi;
	QScriptEngine m_engine;

	static QScriptValue newPushButton(QScriptContext *context, QScriptEngine *engine);
};

#endif