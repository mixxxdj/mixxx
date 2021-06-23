#pragma once

#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "control/controlencoder.h"
#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "controllers/softtakeover.h"
#include "effects/effect.h"
#include "effects/effectparameterslot.h"
#include "effects/effectbuttonparameterslot.h"
#include "util/class.h"

class EffectSlot;
class ControlProxy;


class EffectSlot : public QObject {
    Q_OBJECT
  public:
    EffectSlot(const QString& group,
               const unsigned int iChainNumber,
               const unsigned int iEffectNumber);
    virtual ~EffectSlot();

    // Return the currently loaded effect, if any. If no effect is loaded,
    // returns a null EffectPointer.
    EffectPointer getEffect() const;

    inline bool getEnableState() const {
        return m_pControlEnabled->toBool();
    }

    inline int getEffectSlotNumber() const {
        return m_iEffectNumber;
    }

    unsigned int numParameterSlots() const;
    EffectParameterSlotPointer addEffectParameterSlot();
    EffectParameterSlotPointer getEffectParameterSlot(unsigned int slotNumber);
    EffectParameterSlotPointer getEffectParameterSlotForConfigKey(unsigned int slotNumber);
    inline const QList<EffectParameterSlotPointer>& getEffectParameterSlots() const {
        return m_parameters;
    };

    unsigned int numButtonParameterSlots() const;
    EffectButtonParameterSlotPointer addEffectButtonParameterSlot();
    EffectButtonParameterSlotPointer getEffectButtonParameterSlot(unsigned int slotNumber);
    inline const QList<EffectButtonParameterSlotPointer>& getEffectButtonParameterSlots() const {
        return m_buttonParameters;
    };

    double getMetaParameter() const;

    // ensures that Softtakover is bypassed for the following
    // ChainParameterChange. Uses for testing only
    void syncSofttakeover();

    // Unload the currently loaded effect
    void clear();

    const QString& getGroup() const {
        return m_group;
    }

    QDomElement toXml(QDomDocument* doc) const;
    void loadEffectSlotFromXml(const QDomElement& effectElement);

  public slots:
    // Request that this EffectSlot load the given Effect
    void loadEffect(EffectPointer pEffect, bool adoptMetaknobPosition);
    void setMetaParameter(double v, bool force = false);

    void slotEnabled(double v);
    void slotNextEffect(double v);
    void slotPrevEffect(double v);
    void slotClear(double v);
    void slotEffectSelector(double v);
    void slotEffectEnabledChanged(bool enabled);
    void slotEffectMetaParameter(double v, bool force);

  signals:
    // Indicates that the effect pEffect has been loaded into this
    // EffectSlot. The effectSlotNumber is provided for the convenience of
    // listeners.  pEffect may be an invalid pointer, which indicates that a
    // previously loaded effect was removed from the slot.
    void effectLoaded(EffectPointer pEffect, unsigned int effectSlotNumber);

    // Signal that whoever is in charge of this EffectSlot should load the next
    // Effect into it.
    void nextEffect(unsigned int iChainNumber, unsigned int iEffectNumber,
                    EffectPointer pEffect);

    // Signal that whoever is in charge of this EffectSlot should load the
    // previous Effect into it.
    void prevEffect(unsigned int iChainNumber, unsigned int iEffectNumber,
                    EffectPointer pEffect);

    // Signal that whoever is in charge of this EffectSlot should clear this
    // EffectSlot (by deleting the effect from the underlying chain).
    void clearEffect(unsigned int iEffectNumber);

    void updated();

  private:
    QString debugString() const {
        return QString("EffectSlot(%1)").arg(m_group);
    }

    const unsigned int m_iChainNumber;
    const unsigned int m_iEffectNumber;
    const QString m_group;
    UserSettingsPointer m_pConfig;
    EffectPointer m_pEffect;

    ControlObject* m_pControlLoaded;
    ControlPushButton* m_pControlEnabled;
    ControlObject* m_pControlNumParameters;
    ControlObject* m_pControlNumParameterSlots;
    ControlObject* m_pControlNumButtonParameters;
    ControlObject* m_pControlNumButtonParameterSlots;
    ControlObject* m_pControlNextEffect;
    ControlObject* m_pControlPrevEffect;
    ControlEncoder* m_pControlEffectSelector;
    ControlObject* m_pControlClear;
    ControlPotmeter* m_pControlMetaParameter;
    QList<EffectParameterSlotPointer> m_parameters;
    QList<EffectButtonParameterSlotPointer> m_buttonParameters;

    SoftTakeover* m_pSoftTakeover;

    DISALLOW_COPY_AND_ASSIGN(EffectSlot);
};
