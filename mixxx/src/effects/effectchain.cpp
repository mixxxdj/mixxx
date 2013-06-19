#include <QMutexLocker>

#include "effects/effectchain.h"
#include "sampleutil.h"

EffectChain::EffectChain(QObject* pParent)
        : QObject(),
          m_mutex(QMutex::Recursive),
          m_id(""),
          m_name("") {
}

EffectChain::~EffectChain() {
    qDebug() << debugString() << "destroyed";
}

QString EffectChain::id() const {
    QMutexLocker locker(&m_mutex);
    return m_id;
}

void EffectChain::setId(const QString id) {
    QMutexLocker locker(&m_mutex);
    m_id = id;
}

QString EffectChain::name() const {
    QMutexLocker locker(&m_mutex);
    return m_name;
}

void EffectChain::setName(const QString name) {
    QMutexLocker locker(&m_mutex);
    m_name = name;
}

double EffectChain::parameter() const {
    QMutexLocker locker(&m_mutex);
    return m_dParameter;
}

void EffectChain::setParameter(double dParameter) {
    qDebug() << debugString() << "setParameter" << dParameter;
    QMutexLocker locker(&m_mutex);
    m_dParameter = dParameter;

}

void EffectChain::process(const QString channelId,
                          const CSAMPLE* pInput,
                          CSAMPLE* pOutput,
                          const unsigned int numSamples) {
    qDebug() << debugString() << "process" << channelId << numSamples;
    QMutexLocker locker(&m_mutex);

    // Qt implicitly shared state claims this is fine.
    QList<EffectPointer> effects = m_effects;

    ////////////////////////////////////////////////////////////////////////////
    // AFTER THIS LINE, THE MUTEX IS UNLOCKED. DONT TOUCH ANY MEMBER STATE
    ////////////////////////////////////////////////////////////////////////////
    locker.unlock();

    // foreach (EffectPointer pEffect, effects) {
    //     if (pEffect) {
    //         pEffect->process(channelId, pInput, pOutput, numSamples);
    //     }
    // }
}

void EffectChain::addEffect(EffectPointer pEffect) {
    qDebug() << debugString() << "addEffect";
    QMutexLocker locker(&m_mutex);
    m_effects.append(pEffect);
    locker.unlock();
    emit(updated());
}

unsigned int EffectChain::numEffects() const {
    QMutexLocker locker(&m_mutex);
    return m_effects.size();
}

QList<EffectPointer> EffectChain::getEffects() const {
    QMutexLocker locker(&m_mutex);
    return m_effects;
}

EffectPointer EffectChain::getEffect(unsigned int effectNumber) const {
    QMutexLocker locker(&m_mutex);
    if (effectNumber >= m_effects.size()) {
        qDebug() << debugString() << "WARNING: list index out of bounds for getEffect";
    }
    return m_effects[effectNumber];
}
