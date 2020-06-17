#ifndef CONTROLLERENGINEJSPROXY_H
#define CONTROLLERENGINEJSPROXY_H

#include <QJSValue>
#include <QObject>

class ControllerEngine;

// An object of this class gets exposed to the JS engine, so the methods of this class
// constitute the api that is provided to scripts under "engine" object.
//
// The implementation simply forwards its method calls to the ControllerEngine.
// We cannot expose ControllerEngine directly to the JS engine because the JS engine would take
// ownership of ControllerEngine. This is problematic when we reload a script file, because we
// destroy the existing JS engine to create a new one. Then, since the JS engine owns ControllerEngine
// it will try to delete it. See this Qt bug: https://bugreports.qt.io/browse/QTBUG-41171
class ControllerEngineJSProxy : public QObject {
    Q_OBJECT
  public:
    ControllerEngineJSProxy(ControllerEngine* m_pEngine);

    virtual ~ControllerEngineJSProxy();

    Q_INVOKABLE double getValue(QString group, QString name);
    Q_INVOKABLE void setValue(QString group, QString name, double newValue);
    Q_INVOKABLE double getParameter(QString group, QString name);
    Q_INVOKABLE void setParameter(QString group, QString name, double newValue);
    Q_INVOKABLE double getParameterForValue(QString group, QString name, double value);
    Q_INVOKABLE void reset(QString group, QString name);
    Q_INVOKABLE double getDefaultValue(QString group, QString name);
    Q_INVOKABLE double getDefaultParameter(QString group, QString name);
    Q_INVOKABLE QJSValue makeConnection(QString group, QString name, const QJSValue callback);
    // DEPRECATED: Use makeConnection instead.
    Q_INVOKABLE QJSValue connectControl(QString group, QString name, const QJSValue passedCallback, bool disconnect = false);
    // Called indirectly by the objects returned by connectControl
    Q_INVOKABLE void trigger(QString group, QString name);
    Q_INVOKABLE void log(QString message);
    Q_INVOKABLE int beginTimer(int interval, QJSValue scriptCode, bool oneShot = false);
    Q_INVOKABLE void stopTimer(int timerId);
    Q_INVOKABLE void scratchEnable(int deck, int intervalsPerRev, double rpm, double alpha, double beta, bool ramp = true);
    Q_INVOKABLE void scratchTick(int deck, int interval);
    Q_INVOKABLE void scratchDisable(int deck, bool ramp = true);
    Q_INVOKABLE bool isScratching(int deck);
    Q_INVOKABLE void softTakeover(QString group, QString name, bool set);
    Q_INVOKABLE void softTakeoverIgnoreNextValue(QString group, QString name);
    Q_INVOKABLE void brake(int deck, bool activate, double factor = 1.0, double rate = 1.0);
    Q_INVOKABLE void spinback(int deck, bool activate, double factor = 1.8, double rate = -10.0);
    Q_INVOKABLE void softStart(int deck, bool activate, double factor = 1.0);

  private:
    ControllerEngine* m_pEngine;
};

#endif // CONTROLLERENGINEJSPROXY_H
