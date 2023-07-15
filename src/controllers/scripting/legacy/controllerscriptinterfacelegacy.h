#pragma once

#include <QJSValue>
#include <QObject>

#include "controllers/softtakeover.h"
#include "util/alphabetafilter.h"
#include "util/runtimeloggingcategory.h"

class ControllerScriptEngineLegacy;
class ControlObjectScript;
class ScriptConnection;
class ConfigKey;

/// ControllerScriptInterfaceLegacy is the legacy API for controller scripts to interact
/// with Mixxx. It is inserted into the JS environment as the "engine" object.
class ControllerScriptInterfaceLegacy : public QObject {
    Q_OBJECT
  public:
    ControllerScriptInterfaceLegacy(ControllerScriptEngineLegacy* m_pEngine,
            const RuntimeLoggingCategory& logger);

    virtual ~ControllerScriptInterfaceLegacy();

    Q_INVOKABLE double getValue(const QString& group, const QString& name);
    Q_INVOKABLE void setValue(const QString& group, const QString& name, double newValue);
    Q_INVOKABLE double getParameter(const QString& group, const QString& name);
    Q_INVOKABLE void setParameter(const QString& group, const QString& name, double newValue);
    Q_INVOKABLE double getParameterForValue(
            const QString& group, const QString& name, double value);
    Q_INVOKABLE void reset(const QString& group, const QString& name);
    Q_INVOKABLE double getDefaultValue(const QString& group, const QString& name);
    Q_INVOKABLE double getDefaultParameter(const QString& group, const QString& name);
    Q_INVOKABLE QJSValue makeConnection(const QString& group,
            const QString& name,
            const QJSValue& callback);
    Q_INVOKABLE QJSValue makeUnbufferedConnection(const QString& group,
            const QString& name,
            const QJSValue& callback);
    // DEPRECATED: Use makeConnection instead.
    Q_INVOKABLE QJSValue connectControl(const QString& group,
            const QString& name,
            const QJSValue& passedCallback,
            bool disconnect = false);
    // Called indirectly by the objects returned by connectControl
    Q_INVOKABLE void trigger(const QString& group, const QString& name);
    // DEPRECATED: Use console.log instead.
    Q_INVOKABLE void log(const QString& message);
    Q_INVOKABLE int beginTimer(int interval, QJSValue scriptCode, bool oneShot = false);
    Q_INVOKABLE void stopTimer(int timerId);
    Q_INVOKABLE void scratchEnable(int deck,
            int intervalsPerRev,
            double rpm,
            double alpha,
            double beta,
            bool ramp = true);
    Q_INVOKABLE void scratchTick(int deck, int interval);
    Q_INVOKABLE void scratchDisable(int deck, bool ramp = true);
    Q_INVOKABLE bool isScratching(int deck);
    Q_INVOKABLE void softTakeover(const QString& group, const QString& name, bool set);
    Q_INVOKABLE void softTakeoverIgnoreNextValue(const QString& group, const QString& name);
    Q_INVOKABLE void brake(const int deck,
            bool activate,
            double factor = 1.0,
            const double rate = 1.0);
    Q_INVOKABLE void spinback(const int deck,
            bool activate,
            double factor = 1.8,
            const double rate = -10.0);
    Q_INVOKABLE void softStart(const int deck, bool activate, double factor = 1.0);

    bool removeScriptConnection(const ScriptConnection& conn);
    /// Execute a ScriptConnection's JS callback
    void triggerScriptConnection(const ScriptConnection& conn);

    /// Handler for timers that scripts set.
    virtual void timerEvent(QTimerEvent* event);

  private:
    QJSValue makeConnectionInternal(const QString& group,
            const QString& name,
            const QJSValue& callback,
            bool skipSuperseded = false);
    QHash<ConfigKey, ControlObjectScript*> m_controlCache;
    ControlObjectScript* getControlObjectScript(const QString& group, const QString& name);
    void logOrThrowError(const QString& errorMessage) const;

    SoftTakeoverCtrl m_st;

    struct TimerInfo {
        QJSValue callback;
        bool oneShot;
    };
    QHash<int, TimerInfo> m_timers;

    QVarLengthArray<int> m_intervalAccumulator;
    QVarLengthArray<mixxx::Duration> m_lastMovement;
    QVarLengthArray<double> m_dx, m_rampTo, m_rampFactor;
    QVarLengthArray<bool> m_ramp, m_brakeActive, m_spinbackActive, m_softStartActive;
    QVarLengthArray<AlphaBetaFilter*> m_scratchFilters;
    QHash<int, int> m_scratchTimers;
    /// Applies the accumulated movement to the track speed
    void scratchProcess(int timerId);
    void stopScratchTimer(int timerId);
    bool isDeckPlaying(const QString& group);
    void stopDeck(const QString& group);
    bool isTrackLoaded(const QString& group);
    double getDeckRate(const QString& group);

    ControllerScriptEngineLegacy* m_pScriptEngineLegacy;
    const RuntimeLoggingCategory m_logger;
};
