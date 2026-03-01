#pragma once

#include <QObject>
#include <QString>
#include <optional>

#include "effects/backends/effectmanifestparameter.h"
#include "util/class.h"

class ControlObject;
class ControlPushButton;
class EffectParameter;
class EffectSlot;

/// EffectParameterSlotBase is a wrapper around the parameterX ControlObject.
/// EffectSlot loads/unloads an EffectParameter from the EffectParameterSlotBase.
/// The EffectParameter is responsible for communicating changes in the parameter
/// value to the EngineEffectParameter. The separation of EffectParameter and
/// EffectParameterSlotBase allows EffectSlot to arbitrarily hide and rearrange
/// parameters.
class EffectParameterSlotBase : public QObject {
    Q_OBJECT
  public:
    EffectParameterSlotBase(const QString& group,
            const unsigned int iParameterSlotNumber,
            const EffectParameterType parameterType);

    ~EffectParameterSlotBase() override;

    virtual void loadParameter(EffectParameterPointer pEffectParameter) = 0;

    // Clear the currently loaded effect
    virtual void clear() = 0;

    virtual void syncSofttakeover();

    virtual void onEffectMetaParameterChanged(double parameter, bool force = false);

    QString name() const;
    QString shortName() const;
    QString description() const;
    EffectParameterType parameterType() const;
    EffectManifestParameterPointer getManifest();
    inline bool isLoaded() const {
        return m_pManifestParameter != nullptr;
    }

    int slotNumber() const {
        return m_iParameterSlotNumber;
    }

    virtual void setParameter(double value) = 0;

    std::optional<double> neutralPointOnScale() const {
        if (m_pManifestParameter == nullptr) {
            return std::nullopt;
        }
        return m_pManifestParameter->neutralPointOnScale();
    }

  signals:
    // Signal that indicates that the EffectParameterSlotBase has been updated.
    void updated();
    void valueChanged(double v);

  public slots:
    // Solely for handling control changes
    void slotValueChanged(double v);

  protected:
    const unsigned int m_iParameterSlotNumber;
    QString m_group;
    EffectParameterPointer m_pEffectParameter;
    EffectManifestParameterPointer m_pManifestParameter;
    EffectParameterType m_parameterType;

    // Controls exposed to the rest of Mixxx
    std::unique_ptr<ControlObject> m_pControlLoaded;
    std::unique_ptr<ControlObject> m_pControlType;
    double m_dChainParameter;

    DISALLOW_COPY_AND_ASSIGN(EffectParameterSlotBase);
};
