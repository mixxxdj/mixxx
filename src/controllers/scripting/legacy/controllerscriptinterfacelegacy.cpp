#include "controllerscriptinterfacelegacy.h"

#include <QStringEncoder>
#include <gsl/pointers>

#include "control/controlobject.h"
#include "control/controlobjectscript.h"
#include "control/controlpotmeter.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "controllers/scripting/legacy/scriptconnectionjsproxy.h"
#include "mixer/playermanager.h"
#include "moc_controllerscriptinterfacelegacy.cpp"
#include "util/cmdlineargs.h"
#include "util/fpclassify.h"
#include "util/make_const_iterator.h"
#include "util/time.h"

#define SCRATCH_DEBUG_OUTPUT false

namespace {
constexpr int kDecks = 16;

// Use 1ms for the Alpha-Beta dt. We're assuming the OS actually gives us a 1ms
// timer.
constexpr int kScratchTimerMs = 1;
constexpr double kAlphaBetaDt = kScratchTimerMs / 1000.0;
// stop ramping at a rate which doesn't produce any audible output anymore
constexpr double kBrakeRampToRate = 0.01;
} // namespace

ControllerScriptInterfaceLegacy::ControllerScriptInterfaceLegacy(
        ControllerScriptEngineLegacy* m_pEngine, const RuntimeLoggingCategory& logger)
        : m_pScriptEngineLegacy(m_pEngine),
          m_logger(logger) {
    // Pre-allocate arrays for average number of virtual decks
    m_intervalAccumulator.resize(kDecks);
    m_lastMovement.resize(kDecks);
    m_dx.resize(kDecks);
    m_rampTo.resize(kDecks);
    m_ramp.resize(kDecks);
    m_scratchFilters.resize(kDecks);
    m_rampFactor.resize(kDecks);
    m_brakeActive.resize(kDecks);
    m_spinbackActive.resize(kDecks);
    m_softStartActive.resize(kDecks);
    // Initialize arrays used for testing and pointers
    for (int i = 0; i < kDecks; ++i) {
        m_dx[i] = 0.0;
        m_scratchFilters[i] = new AlphaBetaFilter();
        m_ramp[i] = false;
        m_brakeActive[i] = false;
        m_spinbackActive[i] = false;
        m_softStartActive[i] = false;
    }
}

ControllerScriptInterfaceLegacy::~ControllerScriptInterfaceLegacy() {
    // Stop all timers
    const auto timerIds = m_timers.keys();
    for (const int timerId : timerIds) {
        stopTimer(timerId);
    }

    // Prevents leaving decks in an unstable state
    //  if the controller is shut down while scratching
    QHashIterator<int, int> it(m_scratchTimers);
    while (it.hasNext()) {
        it.next();
        qCDebug(m_logger) << "Aborting scratching on deck" << it.value();
        // Clear scratch2_enable. PlayerManager::groupForDeck is 0-indexed.
        QString group = PlayerManager::groupForDeck(it.value() - 1);
        ControlObjectScript* pScratch2Enable =
                getControlObjectScript(group, "scratch2_enable");
        if (pScratch2Enable != nullptr) {
            pScratch2Enable->set(0);
        }
    }

    for (int i = 0; i < kDecks; ++i) {
        delete m_scratchFilters[i];
        m_scratchFilters[i] = nullptr;
    }

    // Free all the ControlObjectScripts
    {
        auto it = m_controlCache.constBegin();
        while (it != m_controlCache.constEnd()) {
            qCDebug(m_logger)
                    << "Deleting ControlObjectScript"
                    << it.key().group
                    << it.key().item;
            delete it.value();
            // Advance iterator
            it = constErase(&m_controlCache, it);
        }
    }
}

ControlObjectScript* ControllerScriptInterfaceLegacy::getControlObjectScript(
        const QString& group, const QString& name) {
    ConfigKey key = ConfigKey(group, name);
    ControlObjectScript* coScript = m_controlCache.value(key, nullptr);
    if (coScript == nullptr) {
        // create COT
        coScript = new ControlObjectScript(key, m_logger, this);
        if (coScript->valid()) {
            m_controlCache.insert(key, coScript);
        } else {
            delete coScript;
            coScript = nullptr;
        }
    }
    return coScript;
}

QJSValue ControllerScriptInterfaceLegacy::getSetting(const QString& name) {
    VERIFY_OR_DEBUG_ASSERT(m_pScriptEngineLegacy) {
        return QJSValue::UndefinedValue;
    }
    if (name.isEmpty()) {
        m_pScriptEngineLegacy->logOrThrowError(
                QStringLiteral("getSetting called with empty name "
                               "string, returning undefined")
                        .arg(name));
        return QJSValue::UndefinedValue;
    }

    const auto it = m_pScriptEngineLegacy->m_settings.constFind(name);
    if (it != m_pScriptEngineLegacy->m_settings.constEnd()) {
        return it.value();
    } else {
        m_pScriptEngineLegacy->logOrThrowError(
                QStringLiteral("Unknown controllerSetting (%1) returning undefined")
                        .arg(name));
        return QJSValue::UndefinedValue;
    }
}

QObject* ControllerScriptInterfaceLegacy::getPlayer(const QString& group) {
    return m_pScriptEngineLegacy->getPlayer(group);
}

double ControllerScriptInterfaceLegacy::getValue(const QString& group, const QString& name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript == nullptr) {
        m_pScriptEngineLegacy->logOrThrowError(
                QStringLiteral("Unknown control (%1, %2) returning 0.0")
                        .arg(group, name));
        return 0.0;
    }
    return coScript->get();
}

void ControllerScriptInterfaceLegacy::setValue(
        const QString& group, const QString& name, double newValue) {
    if (util_isnan(newValue)) {
        m_pScriptEngineLegacy->logOrThrowError(QStringLiteral(
                "Script tried setting (%1, %2) to NotANumber (NaN)")
                                                       .arg(group, name));
        return;
    }

    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript != nullptr) {
        ControlObject* pControl = ControlObject::getControl(
                coScript->getKey(), ControlFlag::AllowMissingOrInvalid);
        if (pControl &&
                !m_st.ignore(
                        pControl, coScript->getParameterForValue(newValue))) {
            coScript->set(newValue);
        }
    }
}

double ControllerScriptInterfaceLegacy::getParameter(const QString& group, const QString& name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript == nullptr) {
        m_pScriptEngineLegacy->logOrThrowError(
                QStringLiteral("Unknown control (%1, %2) returning 0.0")
                        .arg(group, name));
        return 0.0;
    }
    return coScript->getParameter();
}

void ControllerScriptInterfaceLegacy::setParameter(
        const QString& group, const QString& name, double newParameter) {
    if (util_isnan(newParameter)) {
        m_pScriptEngineLegacy->logOrThrowError(QStringLiteral(
                "Script tried setting (%1, %2) to NotANumber (NaN)")
                                                       .arg(group, name));
        return;
    }

    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript != nullptr) {
        ControlObject* pControl = ControlObject::getControl(
                coScript->getKey(), ControlFlag::AllowMissingOrInvalid);
        if (pControl && !m_st.ignore(pControl, newParameter)) {
            coScript->setParameter(newParameter);
        }
    }
}

double ControllerScriptInterfaceLegacy::getParameterForValue(
        const QString& group, const QString& name, double value) {
    if (util_isnan(value)) {
        m_pScriptEngineLegacy->logOrThrowError(QStringLiteral(
                "Script tried setting (%1, %2) to NotANumber (NaN)")
                                                       .arg(group, name));
        return 0.0;
    }

    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript == nullptr) {
        m_pScriptEngineLegacy->logOrThrowError(
                QStringLiteral("Unknown control (%1, %2) returning 0.0")
                        .arg(group, name));
        return 0.0;
    }

    return coScript->getParameterForValue(value);
}

void ControllerScriptInterfaceLegacy::reset(const QString& group, const QString& name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript != nullptr) {
        coScript->reset();
    }
}

double ControllerScriptInterfaceLegacy::getDefaultValue(const QString& group, const QString& name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript == nullptr) {
        m_pScriptEngineLegacy->logOrThrowError(
                QStringLiteral("Unknown control (%1, %2) returning 0.0")
                        .arg(group, name));
        return 0.0;
    }

    return coScript->getDefault();
}

double ControllerScriptInterfaceLegacy::getDefaultParameter(
        const QString& group, const QString& name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);

    if (coScript == nullptr) {
        m_pScriptEngineLegacy->logOrThrowError(
                QStringLiteral("Unknown control (%1, %2) returning 0.0")
                        .arg(group, name));
        return 0.0;
    }

    return coScript->getParameterForValue(coScript->getDefault());
}

QJSValue ControllerScriptInterfaceLegacy::makeConnection(
        const QString& group, const QString& name, const QJSValue& callback) {
    return ControllerScriptInterfaceLegacy::makeConnectionInternal(group, name, callback, false);
}

QJSValue ControllerScriptInterfaceLegacy::makeUnbufferedConnection(
        const QString& group, const QString& name, const QJSValue& callback) {
    return ControllerScriptInterfaceLegacy::makeConnectionInternal(group, name, callback, true);
}

QJSValue ControllerScriptInterfaceLegacy::makeConnectionInternal(
        const QString& group, const QString& name, const QJSValue& callback, bool skipSuperseded) {
    auto pJsEngine = m_pScriptEngineLegacy->jsEngine();
    VERIFY_OR_DEBUG_ASSERT(pJsEngine) {
        return QJSValue();
    }

    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript == nullptr) {
        // The test setups do not run all of Mixxx, so ControlObjects not
        // existing during tests is okay.
        if (!m_pScriptEngineLegacy->isTesting()) {
            m_pScriptEngineLegacy->logOrThrowError(
                    QStringLiteral("script tried to connect to ControlObject "
                                   "(%1, %2) which is non-existent.")
                            .arg(group, name));
        }
        return QJSValue();
    }

    if (!callback.isCallable()) {
        m_pScriptEngineLegacy->logOrThrowError(QStringLiteral(
                "Tried to connect (%1, %2) to an invalid callback. Make sure "
                "that your code contains no syntax errors.")
                                                       .arg(group, name));
        return QJSValue();
    }

    ScriptConnection connection;
    connection.key = ConfigKey(group, name);
    connection.engineJSProxy = this;
    connection.controllerEngine = m_pScriptEngineLegacy;
    connection.callback = callback;
    connection.id = QUuid::createUuid();
    connection.skipSuperseded = skipSuperseded;

    if (coScript->addScriptConnection(connection)) {
        return pJsEngine->newQObject(
                new ScriptConnectionJSProxy(std::move(connection)));
    }

    return QJSValue();
}

bool ControllerScriptInterfaceLegacy::removeScriptConnection(
        const ScriptConnection& connection) {
    ControlObjectScript* coScript =
            getControlObjectScript(connection.key.group, connection.key.item);

    if (m_pScriptEngineLegacy->jsEngine() == nullptr || coScript == nullptr) {
        return false;
    }

    return coScript->removeScriptConnection(connection);
}

void ControllerScriptInterfaceLegacy::triggerScriptConnection(
        const ScriptConnection& connection) {
    VERIFY_OR_DEBUG_ASSERT(m_pScriptEngineLegacy->jsEngine()) {
        return;
    }

    ControlObjectScript* coScript =
            getControlObjectScript(connection.key.group, connection.key.item);
    if (coScript == nullptr) {
        m_pScriptEngineLegacy->logOrThrowError(QStringLiteral(
                "Script tried to trigger (%1, %2) which is non-existent.")
                                                       .arg(connection.key.group,
                                                               connection.key.item));
        return;
    }

    connection.executeCallback(coScript->get());
}

// This function is a legacy version of makeConnection with several alternate
// ways of invoking it. The callback function can be passed either as a string of
// JavaScript code that evaluates to a function or an actual JavaScript function.
// If "true" is passed as a 4th parameter, all connections to the ControlObject
// are removed. If a ScriptConnectionInvokableWrapper is passed instead of a callback,
// it is disconnected.
// WARNING: These behaviors are quirky and confusing, so if you change this function,
// be sure to run the ControllerScriptInterfaceTest suite to make sure you do not break old scripts.
QJSValue ControllerScriptInterfaceLegacy::connectControl(const QString& group,
        const QString& name,
        const QJSValue& passedCallback,
        bool disconnect) {
    m_pScriptEngineLegacy->logOrThrowError(QStringLiteral(
            "Script tried to connect to (%1, %2) using `connectControl` which "
            "is deprecated. Use `makeConnection` instead!")
                                                   .arg(group, name));

    // The passedCallback may or may not actually be a function, so when
    // the actual callback function is found, store it in this variable.
    QJSValue actualCallbackFunction;

    if (passedCallback.isCallable()) {
        if (!disconnect) {
            // skip all the checks below and just make the connection
            return makeConnection(group, name, passedCallback);
        }
        actualCallbackFunction = passedCallback;
    }

    auto pJsEngine = m_pScriptEngineLegacy->jsEngine();

    ControlObjectScript* coScript = getControlObjectScript(group, name);
    // This check is redundant with makeConnection, but the
    // ControlObjectScript is also needed here to check for duplicate connections.
    if (coScript == nullptr) {
        // The test setups do not run all of Mixxx, so ControlObjects not
        // existing during tests is okay.
        if (!m_pScriptEngineLegacy->isTesting()) {
            if (disconnect) {
                m_pScriptEngineLegacy->throwJSError(
                        "script tried to disconnect from "
                        "ControlObject (" +
                        group + ", " + name + ") which is non-existent.");
            } else {
                m_pScriptEngineLegacy->throwJSError(
                        "script tried to connect to "
                        "ControlObject (" +
                        group + ", " + name + ") which is non-existent.");
            }
        }
        // This is inconsistent with other failures, which return false.
        // QJSValue() with no arguments is undefined in JavaScript.
        return QJSValue();
    }

    if (passedCallback.isString()) {
        // This check is redundant with makeConnection, but it must be done here
        // before evaluating the code string.
        VERIFY_OR_DEBUG_ASSERT(pJsEngine != nullptr) {
            return QJSValue(false);
        }

        actualCallbackFunction =
                pJsEngine->evaluate(passedCallback.toString());

        if (!actualCallbackFunction.isCallable()) {
            QString sErrorMessage(
                    "Invalid connection callback provided to "
                    "engine.connectControl.");
            if (actualCallbackFunction.isError()) {
                sErrorMessage.append("\n" + actualCallbackFunction.toString());
            }
            m_pScriptEngineLegacy->throwJSError(sErrorMessage);
            return QJSValue(false);
        }

        if (coScript->countConnections() > 0 && !disconnect) {
            // This is inconsistent with the behavior when passing the callback as
            // a function, but keep the old behavior to make sure old scripts do
            // not break.
            ScriptConnection connection = coScript->firstConnection();

            qCWarning(m_logger).nospace()
                    << "Tried to make duplicate connection between (" << group
                    << ", " << name << ") and " << passedCallback.toString()
                    << " but this is not allowed when passing a callback "
                       "as a string. If you actually want to create duplicate "
                       "connections, use engine.makeConnection. "
                       "Returning reference to connection "
                    << connection.id.toString();

            return pJsEngine->newQObject(
                    new ScriptConnectionJSProxy(std::move(connection)));
        }
    } else if (passedCallback.isQObject()) {
        // Assume a ScriptConnection and assume that the script author
        // wants to disconnect it, regardless of the disconnect parameter
        // and regardless of whether it is connected to the same ControlObject
        // specified by the first two parameters to this function.
        QObject* qobject = passedCallback.toQObject();
        const QMetaObject* qmeta = qobject->metaObject();

        qCWarning(m_logger) << "QObject passed to engine.connectControl. Assuming it is"
                            << "a connection object to disconnect and returning false.";
        if (!strcmp(qmeta->className(), "ScriptConnectionJSProxy")) {
            ScriptConnectionJSProxy* proxy = (ScriptConnectionJSProxy*)qobject;
            proxy->disconnect();
        }
        return QJSValue(false);
    }

    // Support removing connections by passing "true" as the last parameter
    // to this function, regardless of whether the callback is provided
    // as a function or a string.
    if (disconnect) {
        // There is no way to determine which
        // ScriptConnection to disconnect unless the script calls
        // ScriptConnectionInvokableWrapper::disconnect(), so
        // disconnect all ScriptConnections connected to the
        // callback function, even though there may be multiple connections.
        coScript->disconnectAllConnectionsToFunction(actualCallbackFunction);
        return QJSValue(true);
    }

    // If execution gets this far without returning, make
    // a new connection to actualCallbackFunction.
    return makeConnection(group, name, actualCallbackFunction);
}

void ControllerScriptInterfaceLegacy::trigger(const QString& group, const QString& name) {
    ControlObjectScript* coScript = getControlObjectScript(group, name);
    if (coScript == nullptr) {
        m_pScriptEngineLegacy->logOrThrowError(QStringLiteral(
                "Script tried to trigger (%1, %2) which is non-existent.")
                                                       .arg(group, name));
        return;
    }
    coScript->emitValueChanged();
}

void ControllerScriptInterfaceLegacy::log(const QString& message) {
    m_pScriptEngineLegacy->logOrThrowError(QStringLiteral(
            "`engine.log` is deprecated. Use `console.log` instead!"));
    qCDebug(m_logger) << message;
}
int ControllerScriptInterfaceLegacy::beginTimer(
        int intervalMillis, QJSValue timerCallback, bool oneShot) {
    if (timerCallback.isString()) {
        m_pScriptEngineLegacy->logOrThrowError(
                QStringLiteral("passed a string to `engine.beginTimer`, please "
                               "pass a function instead!"));
        // wrap the code in a function to make the evaluation lazy.
        // otherwise the code would be evaluated immediately instead of after
        // the timer which is obviously undesired and could also cause
        // issues when used recursively.
        timerCallback = m_pScriptEngineLegacy->jsEngine()->evaluate(
                QStringLiteral("()=>%1").arg(timerCallback.toString()));
    } else if (!timerCallback.isCallable()) {
        QString sErrorMessage(
                "Invalid timer callback provided to engine.beginTimer. Valid "
                "callbacks are strings and functions. "
                "Make sure that your code contains no syntax errors.");
        if (timerCallback.isError()) {
            sErrorMessage.append("\n" + timerCallback.toString());
        }
        m_pScriptEngineLegacy->throwJSError(sErrorMessage);
        return 0;
    }

    if (intervalMillis < 20) {
        qCWarning(m_logger) << "Timer request for" << intervalMillis
                            << "ms is too short. Setting to the minimum of 20ms.";
        intervalMillis = 20;
    }

    // This makes use of every QObject's internal timer mechanism. Nice, clean,
    // and simple. See http://doc.trolltech.com/4.6/qobject.html#startTimer for
    // details
    int timerId = startTimer(intervalMillis);
    TimerInfo info;
    info.callback = timerCallback;
    info.oneShot = oneShot;
    m_timers[timerId] = info;
    if (timerId == 0) {
        m_pScriptEngineLegacy->logOrThrowError(QStringLiteral("Script timer could not be created"));
    } else if (oneShot &&
            // FIXME workaround log spam (Github issue to be created)
            CmdlineArgs::Instance().getControllerDebug()) {
        qCDebug(m_logger) << "Starting one-shot timer:" << timerId;
    } else {
        qCDebug(m_logger) << "Starting timer:" << timerId;
    }
    return timerId;
}

void ControllerScriptInterfaceLegacy::stopTimer(int timerId) {
    if (!m_timers.contains(timerId)) {
        m_pScriptEngineLegacy->logOrThrowError(QStringLiteral(
                "Tried to kill Timer \"%1\" that does not exist")
                                                       .arg(timerId));
        return;
    }
    if (CmdlineArgs::Instance().getControllerDebug()) {
        qCDebug(m_logger) << "Killing timer:" << timerId;
    }
    killTimer(timerId);
    m_timers.remove(timerId);
}

void ControllerScriptInterfaceLegacy::stopScratchTimer(int timerId) {
    if (!m_scratchTimers.contains(timerId)) {
        qCWarning(m_logger) << "Killing scratch timer" << timerId << ": That timer does not exist!";
        return;
    }
    qCDebug(m_logger) << "Killing timer:" << timerId;
    killTimer(timerId);
    m_scratchTimers.remove(timerId);
}

void ControllerScriptInterfaceLegacy::timerEvent(QTimerEvent* event) {
    int timerId = event->timerId();

    // See if this is a scratching timer
    if (m_scratchTimers.contains(timerId)) {
        scratchProcess(timerId);
        return;
    }

    auto it = m_timers.constFind(timerId);
    if (it == m_timers.constEnd()) {
        qCWarning(m_logger) << "Timer" << timerId
                            << "fired but there's no function mapped to it!";
        return;
    }

    // NOTE(rryan): Do not assign by reference -- make a copy. I have no idea
    // why but this causes segfaults in ~QScriptValue while scratching if we
    // don't copy here -- even though internalExecute passes the QScriptValues
    // by value. *boggle*
    TimerInfo timerTarget = it.value();
    if (timerTarget.oneShot) {
        stopTimer(timerId);
    }

    m_pScriptEngineLegacy->executeFunction(&timerTarget.callback);
}

void ControllerScriptInterfaceLegacy::softTakeover(
        const QString& group, const QString& name, bool set) {
    ControlObject* pControl = ControlObject::getControl(
            ConfigKey(group, name), ControlFlag::AllowMissingOrInvalid);
    if (set) {
        auto* pControlPotmeter = qobject_cast<ControlPotmeter*>(pControl);
        if (!pControlPotmeter) {
            return;
        }
        m_st.enable(gsl::not_null(pControlPotmeter));
    } else {
        m_st.disable(pControl);
    }
}

void ControllerScriptInterfaceLegacy::softTakeoverIgnoreNextValue(
        const QString& group, const QString& name) {
    ControlObject* pControl = ControlObject::getControl(
            ConfigKey(group, name), ControlFlag::AllowMissingOrInvalid);
    if (!pControl) {
        return;
    }

    m_st.ignoreNext(pControl);
}

bool ControllerScriptInterfaceLegacy::softTakeoverWillIgnore(
        const QString& group, const QString& name, double parameter) {
    ControlObject* pControl = ControlObject::getControl(
            ConfigKey(group, name));
    if (!pControl) {
        return false;
    }

    return m_st.willIgnore(pControl, parameter);
}

double ControllerScriptInterfaceLegacy::getDeckRate(const QString& group) {
    double rate = 0.0;
    ControlObjectScript* pRateRatio =
            getControlObjectScript(group, "rate_ratio");
    if (pRateRatio != nullptr) {
        rate = pRateRatio->get();
    }

    // See if we're in reverse play
    ControlObjectScript* pReverse = getControlObjectScript(group, "reverse");
    if (pReverse != nullptr && pReverse->get() == 1) {
        rate = -rate;
    }
    return rate;
}

bool ControllerScriptInterfaceLegacy::isDeckPlaying(const QString& group) {
    ControlObjectScript* pPlay = getControlObjectScript(group, "play");

    if (pPlay == nullptr) {
        QString error = QString("Could not get ControlObjectScript(%1, play)").arg(group);
        m_pScriptEngineLegacy->scriptErrorDialog(error, error);
        return false;
    }

    return pPlay->toBool();
}

void ControllerScriptInterfaceLegacy::stopDeck(const QString& group) {
    ControlObjectScript* pPlay = getControlObjectScript(group, "play");

    if (pPlay == nullptr) {
        QString error = QString("Could not get ControlObjectScript(%1, play)").arg(group);
        m_pScriptEngineLegacy->scriptErrorDialog(error, error);
        return;
    }

    pPlay->set(0.0);
}

bool ControllerScriptInterfaceLegacy::isTrackLoaded(const QString& group) {
    ControlObjectScript* pTrackLoaded = getControlObjectScript(group, "track_loaded");

    if (pTrackLoaded == nullptr) {
        QString error = QString("Could not get ControlObjectScript(%1, track_loaded)").arg(group);
        m_pScriptEngineLegacy->scriptErrorDialog(error, error);
        return false;
    }

    return pTrackLoaded->toBool();
}

void ControllerScriptInterfaceLegacy::scratchEnable(int deck,
        int intervalsPerRev,
        double rpm,
        double alpha,
        double beta,
        bool ramp) {
    // If we're already scratching this deck, override that with this request
    if (static_cast<bool>(m_dx[deck])) {
        //qCDebug(m_logger) << "Already scratching deck" << deck << ". Overriding.";
        int timerId = m_scratchTimers.key(deck);
        stopScratchTimer(timerId);
    }

    // Controller resolution in intervals per second at normal speed.
    // (rev/min * ints/rev * mins/sec)
    double intervalsPerSecond = (rpm * intervalsPerRev) / 60.0;

    if (intervalsPerSecond == 0.0) {
        qCWarning(m_logger) << "Invalid rpm or intervalsPerRev supplied to "
                               "scratchEnable. Ignoring request.";
        return;
    }

    m_dx[deck] = 1.0 / intervalsPerSecond;
    m_intervalAccumulator[deck] = 0.0;
    m_ramp[deck] = false;
    m_rampFactor[deck] = 0.001;
    m_brakeActive[deck] = false;

    // PlayerManager::groupForDeck is 0-indexed.
    QString group = PlayerManager::groupForDeck(deck - 1);

    // Ramp velocity, default to stopped.
    double initVelocity = 0.0;

    ControlObjectScript* pScratch2Enable =
            getControlObjectScript(group, "scratch2_enable");

    // If ramping is desired, figure out the deck's current speed
    if (ramp) {
        // See if the deck is already being scratched
        if (pScratch2Enable != nullptr && pScratch2Enable->get() == 1) {
            // If so, set the filter's initial velocity to the scratch speed
            ControlObjectScript* pScratch2 =
                    getControlObjectScript(group, "scratch2");
            if (pScratch2 != nullptr) {
                initVelocity = pScratch2->get();
            }
        } else if (isDeckPlaying(group)) {
            // If the deck is playing, set the filter's initial velocity to the
            // playback speed
            initVelocity = getDeckRate(group);
        }
    }

    // Initialize scratch filter
    if (static_cast<bool>(alpha) && static_cast<bool>(beta)) {
        m_scratchFilters[deck]->init(kAlphaBetaDt, initVelocity, alpha, beta);
    } else {
        // Use filter's defaults if not specified
        m_scratchFilters[deck]->init(kAlphaBetaDt, initVelocity);
    }

    // 1ms is shortest possible, OS dependent
    int timerId = startTimer(kScratchTimerMs);

    // Associate this virtual deck with this timer for later processing
    m_scratchTimers[timerId] = deck;

    // Set scratch2_enable
    if (pScratch2Enable != nullptr) {
        pScratch2Enable->set(1);
    }
}

void ControllerScriptInterfaceLegacy::scratchTick(int deck, int interval) {
    m_lastMovement[deck] = mixxx::Time::elapsed();
    m_intervalAccumulator[deck] += interval;
}

void ControllerScriptInterfaceLegacy::scratchProcess(int timerId) {
#if SCRATCH_DEBUG_OUTPUT
    qDebug() << "   .";
    qDebug() << "   ControllerEngine::scratchProcess";
    qDebug() << "   .";
#endif
    const int deck = m_scratchTimers[timerId];
    // PlayerManager::groupForDeck is 0-indexed.
    const QString group = PlayerManager::groupForDeck(deck - 1);
    AlphaBetaFilter* filter = m_scratchFilters[deck];
    if (!filter) {
        qCWarning(m_logger) << "Scratch filter pointer is null on deck" << deck;
        return;
    }

#if SCRATCH_DEBUG_OUTPUT
    const double oldRate = filter->predictedVelocity();
#endif

    // Give the filter a data point:

    // If we're ramping to end scratching and the wheel hasn't been turned very
    // recently (spinback after lift-off,) feed fixed data
    if (m_ramp[deck] && !m_softStartActive[deck] &&
            ((mixxx::Time::elapsed() - m_lastMovement[deck]) >=
                    mixxx::Duration::fromMillis(1))) {
#if SCRATCH_DEBUG_OUTPUT
        qDebug() << "     ramp && !softStart";
#endif
        filter->observation(m_rampTo[deck] * m_rampFactor[deck]);
        // Once this code path is run, latch so it always runs until reset
        //m_lastMovement[deck] += mixxx::Duration::fromSeconds(1);
    } else if (m_softStartActive[deck]) {
#if SCRATCH_DEBUG_OUTPUT
        qDebug() << "     softStart";
#endif
        // pretend we have moved by (desired rate*default distance)
        filter->observation(m_rampTo[deck] * kAlphaBetaDt);
    } else {
#if SCRATCH_DEBUG_OUTPUT
        qDebug() << "     else";
#endif
        // This will (and should) be 0 if no net ticks have been accumulated
        // (i.e. the wheel is stopped)
        filter->observation(m_dx[deck] * m_intervalAccumulator[deck]);
    }

    const double newRate = filter->predictedVelocity();

    // Actually do the scratching
    ControlObjectScript* pScratch2 = getControlObjectScript(group, "scratch2");
    if (pScratch2 == nullptr) {
        return; // abort and maybe it'll work on the next pass
    }
    pScratch2->set(newRate);

    // Reset accumulator
    m_intervalAccumulator[deck] = 0;

#if SCRATCH_DEBUG_OUTPUT
    qDebug() << "     .";
    qDebug() << "     oldRate " << oldRate;
    qDebug() << "     newRate " << newRate;
    qDebug() << "     fabs    " << fabs(trunc((m_rampTo[deck] - newRate) * 100000) / 100000);
    qDebug() << "     .";
#endif
    // End scratching if we're ramping and the current rate is really close to the rampTo value
    if ((m_ramp[deck] && fabs(m_rampTo[deck] - newRate) <= 0.00001) ||
            // or if we brake, spin back or softstart and have crossed over the desired value,
            (m_brakeActive[deck] && newRate < m_rampTo[deck]) ||
            ((m_spinbackActive[deck] || m_softStartActive[deck]) && newRate > m_rampTo[deck]) ||
            // or if the deck was stopped manually during brake or softStart
            ((m_brakeActive[deck] || m_softStartActive[deck]) && (!isDeckPlaying(group))) ||
            // or if there is no track loaded (anymore)
            !isTrackLoaded(group)) {
        // Not ramping no mo'
        m_ramp[deck] = false;

        if (m_brakeActive[deck] || m_spinbackActive[deck]) {
#if SCRATCH_DEBUG_OUTPUT
            qDebug() << "   brake || spinback, stop scratching, stop deck";
#endif
            // If in brake mode, set scratch2 rate to 0 and stop the deck.
            pScratch2->set(0.0);
            stopDeck(group);
        }

        // Clear scratch2_enable to end scratching.
        ControlObjectScript* pScratch2Enable =
                getControlObjectScript(group, "scratch2_enable");
        if (pScratch2Enable == nullptr) {
            return; // abort and maybe it'll work on the next pass
        }
        pScratch2Enable->set(0);

        stopScratchTimer(timerId);

        m_dx[deck] = 0.0;
        m_brakeActive[deck] = false;
        m_spinbackActive[deck] = false;
        m_softStartActive[deck] = false;
#if SCRATCH_DEBUG_OUTPUT
        qDebug() << "   DONE scratching";
        qDebug() << "   .";
#endif
    }
}

void ControllerScriptInterfaceLegacy::scratchDisable(int deck, bool ramp) {
    // PlayerManager::groupForDeck is 0-indexed.
    QString group = PlayerManager::groupForDeck(deck - 1);

    m_rampTo[deck] = 0.0;

    // If no ramping is desired, disable scratching immediately
    if (!ramp) {
        // Clear scratch2_enable
        ControlObjectScript* pScratch2Enable = getControlObjectScript(group, "scratch2_enable");
        if (pScratch2Enable != nullptr) {
            pScratch2Enable->set(0);
        }
        // Can't return here because we need scratchProcess to stop the timer.
        // So it's still actually ramping, we just won't hear or see it.
    } else if (isDeckPlaying(group)) {
        // If so, set the target velocity to the playback speed
        m_rampTo[deck] = getDeckRate(group);
    }

    m_lastMovement[deck] = mixxx::Time::elapsed();
    m_ramp[deck] = true; // Activate the ramping in scratchProcess()
}

bool ControllerScriptInterfaceLegacy::isScratching(int deck) {
    // PlayerManager::groupForDeck is 0-indexed.
    QString group = PlayerManager::groupForDeck(deck - 1);
    return getValue(group, "scratch2_enable") > 0;
}

void ControllerScriptInterfaceLegacy::spinback(
        int deck, bool activate, double factor, double rate) {
    qDebug() << "ControllerEngine::spinback(deck:" << deck << ", activate:" << activate
             << ", factor:" << factor << ", rate:" << rate;
    brake(deck, activate, -factor, rate);
}

void ControllerScriptInterfaceLegacy::brake(int deck, bool activate, double factor, double rate) {
    qDebug() << "ControllerEngine::brake(deck:" << deck << ", activate:" << activate
             << ", factor:" << factor << ", rate:" << rate;
    // PlayerManager::groupForDeck is 0-indexed.
    const QString group = PlayerManager::groupForDeck(deck - 1);
    // enable/disable scratch2 mode
    ControlObjectScript* pScratch2Enable = getControlObjectScript(group, "scratch2_enable");
    if (pScratch2Enable != nullptr) {
        pScratch2Enable->set(activate ? 1 : 0);
    }

    // Used for killing the current timer when both enabling or disabling
    // Don't kill timer yet! This may be a brake init while currently spinning back
    // and we don't want to interrupt that.
    int timerId = m_scratchTimers.key(deck);

    if (!activate) {
        m_brakeActive[deck] = false;
        m_spinbackActive[deck] = false;
        stopScratchTimer(timerId);
        return;
    }
    // Distinguish spinback and brake. Both ramp to a very low rate to avoid a long,
    // inaudible run out. For spinback that rate is negative so we don't cross 0.
    // Spinback and brake also require different handling in scratchProcess.
    double initRate;
    if (rate < -kBrakeRampToRate) { // spinback
        m_spinbackActive[deck] = true;
        m_brakeActive[deck] = false;
        m_rampTo[deck] = -kBrakeRampToRate;
        initRate = rate;
    } else if (rate > kBrakeRampToRate) { // brake
        // It just doesn't make sense to allow brake to interrupt spinback or an
        // already running brake process
        if (m_spinbackActive[deck] || m_brakeActive[deck]) {
            return;
        }
        m_brakeActive[deck] = true;
        m_spinbackActive[deck] = false;
        m_rampTo[deck] = kBrakeRampToRate;
        // Let's fetch the current rate to create a seamless brake process
        initRate = getDeckRate(group);
        // If we are currently softStart'ing adopt the current scratch rate
        if (m_softStartActive[deck]) {
            m_softStartActive[deck] = false;
            AlphaBetaFilter* filter = m_scratchFilters[deck];
            if (filter != nullptr) {
                initRate = filter->predictedVelocity();
            }
        }
    } else { // -kBrakeRampToRate <= rate <= kBrakeRampToRate
        // This filters stopped deck and rare case of very low initial rates
        m_brakeActive[deck] = false;
        m_spinbackActive[deck] = false;
        stopScratchTimer(timerId);
        stopDeck(group);
        return;
    }
    stopScratchTimer(timerId);

    // setup timer and set scratch2
    timerId = startTimer(kScratchTimerMs);
    m_scratchTimers[timerId] = deck;

    ControlObjectScript* pScratch2 = getControlObjectScript(group, "scratch2");
    if (pScratch2 != nullptr) {
        pScratch2->set(initRate);
    }

    // setup the filter with default alpha and beta*factor
    double alphaBrake = 1.0 / 512;
    // avoid decimals for fine adjusting
    if (factor > 1) {
        factor = ((factor - 1) / 10) + 1;
    }
    double betaBrake = ((1.0 / 512) / 1024) * factor; // default*factor
    AlphaBetaFilter* filter = m_scratchFilters[deck];
    if (filter != nullptr) {
        filter->init(kAlphaBetaDt, initRate, alphaBrake, betaBrake);
    }

    // activate the ramping in scratchProcess()
    m_ramp[deck] = true;
}

void ControllerScriptInterfaceLegacy::softStart(int deck, bool activate, double factor) {
    qDebug() << "ControllerEngine::softStart(deck:" << deck << ", activate:" << activate
             << ", factor:" << factor;
    // PlayerManager::groupForDeck is 0-indexed.
    const QString group = PlayerManager::groupForDeck(deck - 1);

    // kill timer when both enabling or disabling
    int timerId = m_scratchTimers.key(deck);
    stopScratchTimer(timerId);

    // enable/disable scratch2 mode
    ControlObjectScript* pScratch2Enable = getControlObjectScript(group, "scratch2_enable");
    if (pScratch2Enable != nullptr) {
        pScratch2Enable->set(activate ? 1 : 0);
    }

    // used in scratchProcess for the different timer behavior we need
    m_softStartActive[deck] = activate;

    if (!activate) {
        return;
    }

    double initRate = 0.0;
    // acquire deck rate
    m_rampTo[deck] = getDeckRate(group);

    // if braking or spinning back, get current rate from filter
    if (m_brakeActive[deck] || m_spinbackActive[deck]) {
        m_brakeActive[deck] = false;
        m_spinbackActive[deck] = false;

        AlphaBetaFilter* filter = m_scratchFilters[deck];
        if (filter != nullptr) {
            initRate = filter->predictedVelocity();
        }
    }

    // setup timer, start playing and set scratch2
    timerId = startTimer(kScratchTimerMs);
    m_scratchTimers[timerId] = deck;

    ControlObjectScript* pPlay = getControlObjectScript(group, "play");
    if (pPlay != nullptr) {
        pPlay->set(1.0);
    }

    ControlObjectScript* pScratch2 = getControlObjectScript(group, "scratch2");
    if (pScratch2 != nullptr) {
        pScratch2->set(initRate);
    }

    // setup the filter like in brake(), with default alpha and beta*factor
    double alphaSoft = 1.0 / 512;
    // avoid decimals for fine adjusting
    if (factor > 1) {
        factor = ((factor - 1) / 10) + 1;
    }
    double betaSoft = ((1.0 / 512) / 1024) * factor; // default: (1.0/512)/1024
    AlphaBetaFilter* filter = m_scratchFilters[deck];
    if (filter != nullptr) { // kAlphaBetaDt = 1/1000 seconds
        filter->init(kAlphaBetaDt, initRate, alphaSoft, betaSoft);
    }

    // activate the ramping in scratchProcess()
    m_ramp[deck] = true;
}

bool ControllerScriptInterfaceLegacy::isBrakeActive(int deck) {
    if (deck >= m_brakeActive.size()) {
        m_pScriptEngineLegacy->logOrThrowError(
                QStringLiteral("isBrakeActive called with invalid deck index %1, "
                               "returning false")
                        .arg(QString::number(deck)));
        return false;
    }
    return m_brakeActive[deck];
}

bool ControllerScriptInterfaceLegacy::isSpinbackActive(int deck) {
    if (deck >= m_spinbackActive.size()) {
        m_pScriptEngineLegacy->logOrThrowError(
                QStringLiteral("isSpinbackActive called with invalid deck index %1, "
                               "returning false")
                        .arg(QString::number(deck)));
        return false;
    }
    return m_softStartActive[deck];
}

bool ControllerScriptInterfaceLegacy::isSoftStartActive(int deck) {
    if (deck >= m_softStartActive.size()) {
        m_pScriptEngineLegacy->logOrThrowError(
                QStringLiteral("isSoftStartActive called with invalid deck index %1, "
                               "returning false")
                        .arg(QString::number(deck)));
        return false;
    }
    return m_softStartActive[deck];
}

QByteArray ControllerScriptInterfaceLegacy::convertCharset(
        const ControllerScriptInterfaceLegacy::Charset targetCharset,
        const QString& value) {
    using enum Charset;
    switch (targetCharset) {
    case ASCII:
        return convertCharsetInternal(QLatin1String("US-ASCII"), value);
    case UTF_8:
        return convertCharsetInternal(QLatin1String("UTF-8"), value);
    case UTF_16LE:
        return convertCharsetInternal(QLatin1String("UTF-16LE"), value);
    case UTF_16BE:
        return convertCharsetInternal(QLatin1String("UTF-16BE"), value);
    case UTF_32LE:
        return convertCharsetInternal(QLatin1String("UTF-32LE"), value);
    case UTF_32BE:
        return convertCharsetInternal(QLatin1String("UTF-32BE"), value);
    case CentralEurope:
        return convertCharsetInternal(QLatin1String("windows-1250"), value);
    case Cyrillic:
        return convertCharsetInternal(QLatin1String("windows-1251"), value);
    case WesternEurope:
        return convertCharsetInternal(QLatin1String("windows-1252"), value);
    case Greek:
        return convertCharsetInternal(QLatin1String("windows-1253"), value);
    case Turkish:
        return convertCharsetInternal(QLatin1String("windows-1254"), value);
    case Hebrew:
        return convertCharsetInternal(QLatin1String("windows-1255"), value);
    case Arabic:
        return convertCharsetInternal(QLatin1String("windows-1256"), value);
    case Baltic:
        return convertCharsetInternal(QLatin1String("windows-1257"), value);
    case Vietnamese:
        return convertCharsetInternal(QLatin1String("windows-1258"), value);
    case Latin9:
        return convertCharsetInternal(QLatin1String("ISO-8859-15"), value);
    case Shift_JIS:
        return convertCharsetInternal(QLatin1String("Shift_JIS"), value);
    case EUC_JP:
        return convertCharsetInternal(QLatin1String("EUC-JP"), value);
    case EUC_KR:
        return convertCharsetInternal(QLatin1String("EUC-KR"), value);
    case Big5_HKSCS:
        return convertCharsetInternal(QLatin1String("Big5-HKSCS"), value);
    case KOI8_U:
        return convertCharsetInternal(QLatin1String("KOI8-U"), value);
    case UCS2:
        return convertCharsetInternal(QLatin1String("ISO-10646-UCS-2"), value);
    case SCSU:
        return convertCharsetInternal(QLatin1String("SCSU"), value);
    case BOCU_1:
        return convertCharsetInternal(QLatin1String("BOCU-1"), value);
    case CESU_8:
        return convertCharsetInternal(QLatin1String("CESU-8"), value);
    case Latin1:
        return convertCharsetInternal(QLatin1String("ISO-8859-1"), value);
    }

    m_pScriptEngineLegacy->logOrThrowError(QStringLiteral("Unknown charset specified"));
    return QByteArray();
}

QByteArray ControllerScriptInterfaceLegacy::convertCharsetInternal(
        QLatin1String targetCharset, const QString& value) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QAnyStringView encoderName = QAnyStringView(targetCharset);
#else
    const char* encoderName = targetCharset.data();
#endif
    QStringEncoder fromUtf16 = QStringEncoder(encoderName);
    if (!fromUtf16.isValid()) {
        m_pScriptEngineLegacy->logOrThrowError(QStringLiteral("Unable to open encoder"));
        return QByteArray();
    }
    return fromUtf16(value);
}
