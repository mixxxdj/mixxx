#include <QMutexLocker>

#include "effects/effectchain.h"
#include "sampleutil.h"

EffectChain::EffectChain(QObject* pParent)
        : QObject(pParent),
          m_bEnabled(false),
          m_id(""),
          m_name(""),
          m_dMix(0),
          m_dParameter(0) {
}

EffectChain::~EffectChain() {
    qDebug() << debugString() << "destroyed";
}

QString EffectChain::id() const {
    return m_id;
}

void EffectChain::setId(const QString& id) {
    m_id = id;
}

QString EffectChain::name() const {
    return m_name;
}

void EffectChain::setName(const QString& name) {
    m_name = name;
}

double EffectChain::parameter() const {
    return m_dParameter;
}

void EffectChain::setParameter(const double& dParameter) {
    m_dParameter = dParameter;
}

void EffectChain::addEffect(EffectPointer pEffect) {
    qDebug() << debugString() << "addEffect";
    m_effects.append(pEffect);
    emit(updated());
}

unsigned int EffectChain::numEffects() const {
    return m_effects.size();
}

QList<EffectPointer> EffectChain::getEffects() const {
    return m_effects;
}

EffectPointer EffectChain::getEffect(unsigned int effectNumber) const {
    if (effectNumber >= m_effects.size()) {
        qDebug() << debugString() << "WARNING: list index out of bounds for getEffect";
    }
    return m_effects[effectNumber];
}
