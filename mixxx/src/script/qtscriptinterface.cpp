#include "qtscriptinterface.h"
#include <QPushButton>

QtScriptInterface::QtScriptInterface(PlayInterface* pi) : m_pi(pi),
		m_engine() {
	
	//QPushButton* steve = new QPushButton("Hello!", 0);
	//steve->setVisible(true);
    QScriptValue globalObject = m_engine.globalObject();

	QScriptValue pbfunc = m_engine.newFunction(newPushButton);
	//fun.setProperty(QString("functionName"), engine->scriptValue("QPushButton"))
	globalObject.setProperty("QPushButton", pbfunc);
	globalObject.setProperty("Mixxx", m_engine.newQObject(this));

	//qWarning() << "Script: " << m_engine.evaluate("var bob = new QPushButton(\"Hello, world!\", 0); bob.setText(\"Hello!\"); bob.show()").toString();
	//qWarning() << "Test: " << m_engine.evaluate("Mixxx.test()").toString();
	//qWarning() << "Test2: " << m_engine.evaluate("Mixxx.getValue(\"[Master]\", \"crossfader\")").toString();
}

void QtScriptInterface::executeScript(const char* script, int process) {
	m_pi->setProcess(process);
	m_result = m_engine.evaluate(script).toString();
	qWarning() << m_result;
	m_pi->clearProcess();
}

QString QtScriptInterface::getResult() {
	return m_result;
}

QScriptValue QtScriptInterface::newPushButton(QScriptContext *context, QScriptEngine *engine)
{
    return engine->newQObject(new QPushButton());
}

void QtScriptInterface::test() {
	m_pi->test();
}

void QtScriptInterface::stop(int channel) {
	m_pi->stop(channel);
}

void QtScriptInterface::play(int channel) {
	m_pi->play(channel);
}

void QtScriptInterface::setTag(int tag) {
	m_pi->setTag(tag);
}

void QtScriptInterface::clearTag() {
	m_pi->clearTag();
}
	
void QtScriptInterface::kill() {
	m_pi->kill();
}

void QtScriptInterface::killTag(int tag) {
	m_pi->killTag(tag);
}
		
double QtScriptInterface::getValue(QString group, QString name) {
	return m_pi->getValue(group, name);
}

void QtScriptInterface::startList(QString group, QString name) {
	m_pi->startList(group, name);
}

void QtScriptInterface::startFade(QString group, QString name) {
	m_pi->startFade(group, name);
}

void QtScriptInterface::point(int time, double value) {
	m_pi->fadePoint(time, value);
}

void QtScriptInterface::endFade() {
	m_pi->endFade();
}

void QtScriptInterface::endList() {
	m_pi->endList();
}

void QtScriptInterface::playChannel1(int time, QString path) {
	m_pi->playChannel1(time, path);
}

void QtScriptInterface::playChannel2(int time, QString path) {
	m_pi->playChannel2(time, path);
}