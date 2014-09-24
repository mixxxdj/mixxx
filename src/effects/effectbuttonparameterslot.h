#ifndef EFFECTBUTTONPARAMETERSLOT_H
#define EFFECTBUTTONPARAMETERSLOT_H

#include <QObject>
#include <QVariant>
#include <QString>

#include "util.h"
#include "controlobject.h"
#include "effects/effect.h"
#include "effects/effectparameterslotbase.h"

class ControlObject;
class ControlPushButton;

class EffectButtonParameterSlot;
typedef QSharedPointer<EffectButtonParameterSlot> EffectButtonParameterSlotPointer;

class EffectButtonParameterSlot : public EffectParameterSlotBase {
    Q_OBJECT
  public:
    EffectButtonParameterSlot(const unsigned int iRackNumber,
                        const unsigned int iChainNumber,
                        const unsigned int iSlotNumber,
                        const unsigned int iParameterSlotNumber);
    virtual ~EffectButtonParameterSlot();

    static QString formatItemPrefix(const unsigned int iParameterSlotNumber) {
        return QString("button_parameter%1").arg(iParameterSlotNumber + 1);
    }

    // Load the parameter of the given effect into this EffectButtonParameterSlot
    void loadEffect(EffectPointer pEffect);

    void onChainParameterChanged(double parameter);

  private slots:
    // Solely for handling control changes
    void slotParameterValueChanged(QVariant value);
    void slotValueChanged(double v);

  private:
    QString debugString() const {
        return QString("EffectButtonParameterSlot(%1,%2)").arg(m_group).arg(m_iParameterSlotNumber);
    }

    // Clear the currently loaded effect
    void clear();

    // Control exposed to the rest of Mixxx
    ControlPushButton* m_pControlValue;

    DISALLOW_COPY_AND_ASSIGN(EffectButtonParameterSlot);
};

#endif // EFFECTBUTTONPARAMETERSLOT_H
