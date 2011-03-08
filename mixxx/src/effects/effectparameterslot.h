#ifndef EFFECTPARAMETERSLOT_H
#define EFFECTPARAMETERSLOT_H

#include <QObject>
#include <QMutex>
#include <QVariant>
#include <QString>

#include "util.h"
#include "controlobject.h"
#include "effects/effect.h"

class EffectParameterSlot : QObject {
    Q_OBJECT
  public:
    EffectParameterSlot(QObject* pParent, const unsigned int iChainNumber,
                        const unsigned int iSlotNumber, const unsigned int iParameterNumber);
    virtual ~EffectParameterSlot();

    static QString formatGroupString(const unsigned int iChainNumber, const unsigned int iSlotNumber,
                              const unsigned int iParameterNumber) {
        return QString("[EffectChain%1_Effect%2_Parameter%3]")
                .arg(iChainNumber+1)
                .arg(iSlotNumber+1)
                .arg(iParameterNumber+1);
    }

    // Load the parameter of the given effect into this EffectParameterSlot
    void loadEffect(EffectPointer pEffect);

  private slots:
    // Solely for handling control changes
    void slotEnabled(double v);
    void slotLinked(double v);
    void slotValue(double v);
    void slotValueNormalized(double v);
    void slotValueType(double v);
    void slotValueDefault(double v);
    void slotValueMaximum(double v);
    void slotValueMaximumLimit(double v);
    void slotValueMinimum(double v);
    void slotValueMinimumLimit(double v);

  private:
    QString debugString() const {
        return QString("EffectParameterSlot(%1,%2)").arg(m_group).arg(m_iParameterNumber);
    }

    // Clear the currently loaded effect
    void clear();

    mutable QMutex m_mutex;
    const unsigned int m_iChainNumber;
    const unsigned int m_iSlotNumber;
    const unsigned int m_iParameterNumber;
    const QString m_group;
    EffectParameterPointer m_pEffectParameter;

    ////////////////////////////////////////////////////////////////////////////////
    // Controls exposed to the rest of Mixxx
    ////////////////////////////////////////////////////////////////////////////////

    ControlObject* m_pControlEnabled;
    ControlObject* m_pControlLinked;
    ControlObject* m_pControlValue;
    ControlObject* m_pControlValueNormalized;
    ControlObject* m_pControlValueType;
    ControlObject* m_pControlValueDefault;
    ControlObject* m_pControlValueMaximum;
    ControlObject* m_pControlValueMaximumLimit;
    ControlObject* m_pControlValueMinimum;
    ControlObject* m_pControlValueMinimumLimit;

    DISALLOW_COPY_AND_ASSIGN(EffectParameterSlot);
};

#endif /* EFFECTPARAMETERSLOT_H */
