#include "controllerenginejsproxy.h"

#include "controllers/engine/controllerengine.h"
#include "moc_controllerenginejsproxy.cpp"

ControllerEngineJSProxy::ControllerEngineJSProxy(ControllerEngine* m_pEngine)
        : m_pEngine(m_pEngine) {
}

ControllerEngineJSProxy::~ControllerEngineJSProxy() {
}

double ControllerEngineJSProxy::getValue(
        const QString& group,
        const QString& name) {
    return m_pEngine->getValue(group, name);
}

void ControllerEngineJSProxy::setValue(
        const QString& group,
        const QString& name,
        double newValue) {
    m_pEngine->setValue(group, name, newValue);
}

double ControllerEngineJSProxy::getParameter(
        const QString& group,
        const QString& name) {
    return m_pEngine->getParameter(group, name);
}

void ControllerEngineJSProxy::setParameter(
        const QString& group,
        const QString& name,
        double newValue) {
    m_pEngine->setParameter(group, name, newValue);
}

double ControllerEngineJSProxy::getParameterForValue(
        const QString& group,
        const QString& name,
        double value) {
    return m_pEngine->getParameterForValue(group, name, value);
}

void ControllerEngineJSProxy::reset(
        const QString& group,
        const QString& name) {
    m_pEngine->reset(group, name);
}

double ControllerEngineJSProxy::getDefaultValue(
        const QString& group,
        const QString& name) {
    return m_pEngine->getDefaultValue(group, name);
}

double ControllerEngineJSProxy::getDefaultParameter(
        const QString& group,
        const QString& name) {
    return m_pEngine->getDefaultParameter(group, name);
}

QJSValue ControllerEngineJSProxy::makeConnection(
        const QString& group,
        const QString& name,
        const QJSValue& callback) {
    return m_pEngine->makeConnection(group, name, callback);
}

QJSValue ControllerEngineJSProxy::connectControl(
        const QString& group,
        const QString& name,
        const QJSValue& passedCallback,
        bool disconnect) {
    return m_pEngine->connectControl(group, name, passedCallback, disconnect);
}

void ControllerEngineJSProxy::trigger(
        const QString& group,
        const QString& name) {
    m_pEngine->trigger(group, name);
}

void ControllerEngineJSProxy::log(const QString& message) {
    m_pEngine->log(message);
}

int ControllerEngineJSProxy::beginTimer(
        int interval,
        const QJSValue& scriptCode,
        bool oneShot) {
    return m_pEngine->beginTimer(interval, scriptCode, oneShot);
}

void ControllerEngineJSProxy::stopTimer(int timerId) {
    m_pEngine->stopTimer(timerId);
}

void ControllerEngineJSProxy::scratchEnable(
        int deck,
        int intervalsPerRev,
        double rpm,
        double alpha,
        double beta,
        bool ramp) {
    m_pEngine->scratchEnable(deck, intervalsPerRev, rpm, alpha, beta, ramp);
}

void ControllerEngineJSProxy::scratchTick(int deck, int interval) {
    m_pEngine->scratchTick(deck, interval);
}

void ControllerEngineJSProxy::scratchDisable(int deck, bool ramp) {
    m_pEngine->scratchDisable(deck, ramp);
}

bool ControllerEngineJSProxy::isScratching(int deck) {
    return m_pEngine->isScratching(deck);
}

void ControllerEngineJSProxy::softTakeover(
        const QString& group,
        const QString& name,
        bool set) {
    m_pEngine->softTakeover(group, name, set);
}

void ControllerEngineJSProxy::softTakeoverIgnoreNextValue(
        const QString& group,
        const QString& name) {
    m_pEngine->softTakeoverIgnoreNextValue(group, name);
}

void ControllerEngineJSProxy::brake(
        int deck,
        bool activate,
        double factor,
        double rate) {
    m_pEngine->brake(deck, activate, factor, rate);
}

void ControllerEngineJSProxy::spinback(
        int deck,
        bool activate,
        double factor,
        double rate) {
    m_pEngine->spinback(deck, activate, factor, rate);
}

void ControllerEngineJSProxy::softStart(int deck, bool activate, double factor) {
    m_pEngine->softStart(deck, activate, factor);
}
