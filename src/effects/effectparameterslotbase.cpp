#include <QtDebug>

#include "controleffectknob.h"
#include "effects/effectparameterslotbase.h"
#include "controlobject.h"
#include "controlpushbutton.h"

EffectParameterSlotBase::EffectParameterSlotBase(const QString& group,
                                                 const unsigned int iParameterSlotNumber)
        : m_iParameterSlotNumber(iParameterSlotNumber),
          m_group(group),
          m_pEffectParameter(NULL),
          m_dChainParameter(0.0) {

}

EffectParameterSlotBase::~EffectParameterSlotBase() {
    m_pEffectParameter = NULL;
    m_pEffect.clear();
    delete m_pControlLoaded;
    delete m_pControlType;
}

QString EffectParameterSlotBase::name() const {
    if (m_pEffectParameter) {
        return m_pEffectParameter->name();
    }
    return QString();
}

QString EffectParameterSlotBase::description() const {
    if (m_pEffectParameter) {
        return m_pEffectParameter->description();
    }
    return tr("No effect loaded.");
}

void EffectParameterSlotBase::slotLoaded(double v) {
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotLoaded" << v;
    qWarning() << "WARNING: loaded is a read-only control.";
}

void EffectParameterSlotBase::slotValueType(double v) {
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotValueType" << v;
    qWarning() << "WARNING: value_type is a read-only control.";
}

const EffectManifestParameter EffectParameterSlotBase::getManifest() {
    if (m_pEffectParameter) {
        return m_pEffectParameter->manifest();
    }
    return EffectManifestParameter();
}
