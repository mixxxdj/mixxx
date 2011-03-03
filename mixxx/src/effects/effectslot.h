#ifndef EFFECTSLOT_H
#define EFFECTSLOT_H

#include <QMutex>
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
    EffectSlot(QObject* pParent, const unsigned int iChainNumber, const unsigned int iSlotNumber);
    virtual ~EffectSlot();

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
    DISALLOW_COPY_AND_ASSIGN(EffectSlot);
    QString debugString() const {
        return QString("EffectChain(%1)").arg(m_iChainNumber);
    }

    // Unload the currently loaded effect
    void clear();

    mutable QMutex m_mutex;
    const unsigned int m_iChainNumber;
    const unsigned int m_iSlotNumber;
    const QString m_group;
    EffectPointer m_pEffect;

    ControlObject* m_pControlEnabled;
    ControlObject* m_pControlNumParameters;
    QList<EffectParameterSlot*> m_parameters;
};

#endif /* EFFECTSLOT_H */
