#include <QtDebug>

#include "controleffectknob.h"
#include "effects/effectparameterslotbase.h"
#include "controlobject.h"
#include "controlpushbutton.h"

EffectParameterSlotBase::EffectParameterSlotBase(const unsigned int iRackNumber,
                                         const unsigned int iChainNumber,
                                         const unsigned int iSlotNumber,
                                         const unsigned int iParameterSlotNumber)
        : m_iRackNumber(iRackNumber),
          m_iChainNumber(iChainNumber),
          m_iSlotNumber(iSlotNumber),
          m_iParameterSlotNumber(iParameterSlotNumber),
          m_group(formatGroupString(m_iRackNumber, m_iChainNumber,
                                    m_iSlotNumber)),
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
