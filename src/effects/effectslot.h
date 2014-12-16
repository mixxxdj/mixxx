#ifndef EFFECTSLOT_H
#define EFFECTSLOT_H

#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "util.h"
#include "controlobject.h"
#include "controlpushbutton.h"
#include "effects/effect.h"
#include "effects/effectparameterslot.h"
#include "effects/effectbuttonparameterslot.h"

class EffectSlot;
class ControlObjectSlave;
typedef QSharedPointer<EffectSlot> EffectSlotPointer;

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

    unsigned int numParameterSlots() const;
    EffectParameterSlotPointer addEffectParameterSlot();
    EffectParameterSlotPointer getEffectParameterSlot(unsigned int slotNumber);

    unsigned int numButtonParameterSlots() const;
    EffectButtonParameterSlotPointer addEffectButtonParameterSlot();
    EffectButtonParameterSlotPointer getEffectButtonParameterSlot(unsigned int slotNumber);

    void onChainSuperParameterChanged(double parameter, bool force=false);

    // ensures that Softtakover is bypassed for the following
    // ChainParameterChange. Uses for testing only
    void syncSofttakeover();

    // Unload the currently loaded effect
    void clear();

    const QString& getGroup() const {
        return m_group;
    }

  public slots:
    // Request that this EffectSlot load the given Effect
    void loadEffect(EffectPointer pEffect);

    void slotLoaded(double v);
    void slotNumParameters(double v);
    void slotNumParameterSlots(double v);
    void slotEnabled(double v);
    void slotNextEffect(double v);
    void slotPrevEffect(double v);
    void slotClear(double v);
    void slotEffectSelector(double v);
    void slotEffectEnabledChanged(bool enabled);

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
    EffectPointer m_pEffect;

    ControlObject* m_pControlLoaded;
    ControlPushButton* m_pControlEnabled;
    ControlObject* m_pControlNumParameters;
    ControlObject* m_pControlNumParameterSlots;
    ControlObject* m_pControlNumButtonParameters;
    ControlObject* m_pControlNumButtonParameterSlots;
    ControlObject* m_pControlNextEffect;
    ControlObject* m_pControlPrevEffect;
    ControlObject* m_pControlEffectSelector;
    ControlObject* m_pControlClear;
    QList<EffectParameterSlotPointer> m_parameters;
    QList<EffectButtonParameterSlotPointer> m_buttonParameters;

    DISALLOW_COPY_AND_ASSIGN(EffectSlot);
};

#endif /* EFFECTSLOT_H */
