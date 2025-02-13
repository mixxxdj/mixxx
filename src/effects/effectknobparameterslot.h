#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>

#include "controllers/softtakeover.h"
#include "effects/effectparameterslotbase.h"
#include "util/class.h"

class ControlPushButton;
class ControlEffectKnob;

/// Refer to EffectParameterSlotBase for documentation
class EffectKnobParameterSlot : public EffectParameterSlotBase {
    Q_OBJECT
  public:
    EffectKnobParameterSlot(const QString& group, const unsigned int iParameterSlotNumber);
    ~EffectKnobParameterSlot() override;

    static QString formatItemPrefix(const unsigned int iParameterSlotNumber) {
        return QString("parameter%1").arg(iParameterSlotNumber + 1);
    }

    void loadParameter(EffectParameterPointer pEffectParameter) override;

    double getValueParameter() const;

    void onEffectMetaParameterChanged(double parameter, bool force = false) override;

    // Syncs the Super button with the parameter, that the following
    // super button change will be passed to the effect parameter
    // used during test
    void syncSofttakeover() override;

    // Clear the currently loaded effect
    void clear() override;

    void setParameter(double value) override;

  private slots:
    // Solely for handling control changes
    void slotLinkTypeChanging(double v);
    void slotLinkInverseChanged(double v);

  private:
    QString debugString() const {
        return QString("EffectKnobParameterSlot(%1,%2)").arg(m_group).arg(m_iParameterSlotNumber);
    }

    SoftTakeover m_metaknobSoftTakeover;

    // Control exposed to the rest of Mixxx
    std::unique_ptr<ControlEffectKnob> m_pControlValue;
    std::unique_ptr<ControlPushButton> m_pControlLinkType;
    std::unique_ptr<ControlPushButton> m_pControlLinkInverse;

    DISALLOW_COPY_AND_ASSIGN(EffectKnobParameterSlot);
};
