#ifndef EFFECTSLOT_H
#define EFFECTSLOT_H

#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "util.h"
#include "controlobject.h"
#include "effects/effect.h"
#include "effects/effectparameterslot.h"

class EffectSlot;
typedef QSharedPointer<EffectSlot> EffectSlotPointer;

class EffectSlot : public QObject {
    Q_OBJECT
  public:
    EffectSlot(QObject* pParent,
               const unsigned int iRackNumber,
               const unsigned int iChainNumber,
               const unsigned int iSlotNumber);
    virtual ~EffectSlot();

    static QString formatGroupString(const unsigned int iRackNumber,
                                     const unsigned int iChainNumber,
                                     const unsigned int iSlotNumber) {
        return QString("[EffectRack%1_EffectChain%2_Effect%3]").arg(
            QString::number(iRackNumber+1),
            QString::number(iChainNumber+1),
            QString::number(iSlotNumber+1));

    }

    // Return the currently loaded effect, if any. If no effect is loaded,
    // returns a null EffectPointer.
    EffectPointer getEffect() const;

    unsigned int numParameterSlots() const;
    EffectParameterSlotPointer getEffectParameterSlot(unsigned int slotNumber);

  public slots:
    // Request that this EffectSlot load the given Effect
    void loadEffect(EffectPointer pEffect);

  signals:
    // Indicates that the effect pEffect has been loaded into this
    // EffectSlot. The slotNumber is provided for the convenience of listeners.
    // pEffect may be an invalid pointer, which indicates that a previously
    // loaded effect was removed from the slot.
    void effectLoaded(EffectPointer pEffect, unsigned int slotNumber);

  private:
    QString debugString() const {
        return QString("EffectSlot(%1,%2)").arg(m_iChainNumber).arg(m_iSlotNumber);
    }

    void addEffectParameterSlot();

    // Unload the currently loaded effect
    void clear();

    const unsigned int m_iRackNumber;
    const unsigned int m_iChainNumber;
    const unsigned int m_iSlotNumber;
    const QString m_group;
    EffectPointer m_pEffect;

    ControlObject* m_pControlEnabled;
    ControlObject* m_pControlNumParameters;
    QList<EffectParameterSlotPointer> m_parameters;

    DISALLOW_COPY_AND_ASSIGN(EffectSlot);
};

#endif /* EFFECTSLOT_H */
