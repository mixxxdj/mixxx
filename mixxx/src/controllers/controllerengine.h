/**
* @file controllerengine.h
* @author Sean M. Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief The script engine for use by a Controller.
*/

#ifndef CONTROLLERENGINE_H
#define CONTROLLERENGINE_H

#include <QEvent>
#include <QtScript>
#include <QMessageBox>

#include "configobject.h"
#include "controllers/pitchfilter.h"
#include "controllers/softtakeover.h"
#include "qtscript-bytearray/bytearrayclass.h"

// Forward declaration(s)
class Controller;
class ControlObjectThread;

class ControllerEngine : public QObject {
    Q_OBJECT
  public:
    ControllerEngine(Controller* controller);
    virtual ~ControllerEngine();

    bool isReady();
    bool hasErrors(QString filename);
    const QStringList getErrors(QString filename);

    void setDebug(bool bDebug) {
        m_bDebug = bDebug;
    }

    void setPopups(bool bPopups) {
        m_bPopups = bPopups;
    }

    // Look up registered script functions
    QStringList getScriptFunctions();

    // Look up registered script function prefixes
    QList<QString>& getScriptFunctionPrefixes() { return m_scriptFunctionPrefixes; };

  protected:
    Q_INVOKABLE double getValue(QString group, QString name);
    Q_INVOKABLE void setValue(QString group, QString name, double newValue);
    Q_INVOKABLE bool connectControl(QString group, QString name,
                                    QString function, bool disconnect = false);
    Q_INVOKABLE void trigger(QString group, QString name);
    Q_INVOKABLE void log(QString message);
    Q_INVOKABLE int beginTimer(int interval, QString scriptCode, bool oneShot = false);
    Q_INVOKABLE void stopTimer(int timerId);
    Q_INVOKABLE void scratchEnable(int deck, int intervalsPerRev, float rpm,
                                   float alpha, float beta, bool ramp = true);
    Q_INVOKABLE void scratchTick(int deck, int interval);
    Q_INVOKABLE void scratchDisable(int deck, bool ramp = true);
    Q_INVOKABLE void softTakeover(QString group, QString name, bool set);

    // Handler for timers that scripts set.
    virtual void timerEvent(QTimerEvent *event);

  public slots:
    void slotValueChanged(double value);
    // Evaluate a script file
    bool evaluate(QString filepath);
    // Execute a particular function
    bool execute(QString function);
    // Execute a particular function with a list of arguments
    bool execute(QString function, QScriptValueList args);
    // Execute a particular function with a data string (e.g. a device ID)
    bool execute(QString function, QString data);
    // Execute a particular function with a data buffer
    bool execute(QString function, const QByteArray data);
    void loadScriptFiles(QString configPath,
                         QList<QString> scriptFileNames);
    void initializeScripts(const QList<QString> scriptFunctionPrefixes);
    void gracefulShutdown();

  signals:
    void initialized();
    void resetController();

  private slots:
    void errorDialogButton(QString key, QMessageBox::StandardButton button);

  private:
    bool evaluate(QString scriptName, QList<QString> scriptPaths);
    bool internalExecute(QString scriptCode);
    void initializeScriptEngine();

    void scriptErrorDialog(QString detailedError);
    void generateScriptFunctions(QString code);
    void stopAllTimers();

    bool checkException();
    QScriptEngine *m_pEngine;

    ControlObjectThread* getControlObjectThread(QString group, QString name);

    // Scratching functions & variables
    void scratchProcess(int timerId);

    Controller* m_pController;
    bool m_bDebug;
    bool m_bPopups;
    QMultiHash<ConfigKey, QString> m_connectedControls;
    QStringList m_scriptFunctions;
    QList<QString> m_scriptFunctionPrefixes;
    QMap<QString,QStringList> m_scriptErrors;
    QHash<ConfigKey, ControlObjectThread*> m_controlCache;
    QHash<int, QPair<QString, bool> > m_timers;
    SoftTakeover m_st;
    ByteArrayClass *m_pBaClass;
    // 256 (default) available virtual decks is enough I would think.
    //  If more are needed at run-time, these will move to the heap automatically
    QVarLengthArray<int> m_intervalAccumulator;
    QVarLengthArray<float> m_dx, m_rampTo;
    QVarLengthArray<bool> m_ramp;
    QVarLengthArray<PitchFilter*> m_pitchFilter;
    QHash<int, int> m_scratchTimers;
};

#endif
