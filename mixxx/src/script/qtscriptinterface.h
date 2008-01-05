#ifndef QTSCRIPTINTERFACE_H
#define QTSCRIPTINTERFACE_H

#include "playinterface.h"

#include <QtScript>

class QtScriptInterface : public QObject {

	Q_OBJECT

public:
	QtScriptInterface(PlayInterface *pi);

	QString getResult();

	void executeScript(const char* script, int process);
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
	double getValue(QString group, QString name);
		
//	void startFadeCrossfader();
	void startList(QString group, QString name);
	void startFade(QString group, QString name);
	void point(int time, double value);
//	void fadePoint(int time, double value);
	void endFade();
	void endList();

	void playChannel1(int time, QString path);
	void playChannel2(int time, QString path);

private:
	PlayInterface* m_pi;
	QScriptEngine m_engine;
	QString m_result;

	static QScriptValue newPushButton(QScriptContext *context, QScriptEngine *engine);
};

#endif