#ifndef EFFECTKNOBPARAMETERSLOT_H
#define EFFECTKNOBPARAMETERSLOT_H

#include <QObject>
#include <QVariant>
#include <QString>

#include "control/controlobject.h"
#include "effects/effectparameterslotbase.h"
#include "util/class.h"

class ControlObject;
class ControlPushButton;
class ControlEffectKnob;
class SoftTakeover;
class EffectSlot;

// EffectKnobParameterSlot is a wrapper around the parameterX ControlObject
// that loaded with an EffectParameter into itself by the EffectSlot.
class EffectKnobParameterSlot : public EffectParameterSlotBase {
    Q_OBJECT
  public:
    EffectKnobParameterSlot(const QString& group, const unsigned int iParameterSlotNumber);
    virtual ~EffectKnobParameterSlot();

    static QString formatItemPrefix(const unsigned int iParameterSlotNumber) {
        return QString("parameter%1").arg(iParameterSlotNumber + 1);
    }

    // Load the parameter of the given effect into this EffectKnobParameterSlot
    void loadParameter(EffectParameter* pEffectParameter);

    double getValueParameter() const;

    void onEffectMetaParameterChanged(double parameter, bool force=false);

    // Syncs the Super button with the parameter, that the following
    // super button change will be passed to the effect parameter
    // used during test
    void syncSofttakeover();

    // Clear the currently loaded effect
    void clear();

    QDomElement toXml(QDomDocument* doc) const override;
    void loadParameterSlotFromXml(const QDomElement& parameterElement) override;

  private slots:
    // Solely for handling control changes
    void slotParameterValueChanged(double value);
    void slotValueChanged(double v);
    void slotLinkTypeChanging(double v);
    void slotLinkInverseChanged(double v);

  private:
    QString debugString() const {
        return QString("EffectKnobParameterSlot(%1,%2)").arg(m_group).arg(m_iParameterSlotNumber);
    }

    SoftTakeover* m_pSoftTakeover;

    // Control exposed to the rest of Mixxx
    ControlEffectKnob* m_pControlValue;
    ControlPushButton* m_pControlLinkType;
    ControlPushButton* m_pControlLinkInverse;

    DISALLOW_COPY_AND_ASSIGN(EffectKnobParameterSlot);
};

#endif // EFFECTKNOBPARAMETERSLOT_H
