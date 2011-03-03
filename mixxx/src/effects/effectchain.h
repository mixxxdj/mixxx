#ifndef EFFECTCHAIN_H
#define EFFECTCHAIN_H

#include <QObject>
#include <QMutex>
#include <QList>

#include "util.h"
#include "effects/effect.h"
#include "effects/effectslot.h"

class EffectChain;
typedef QSharedPointer<EffectChain> EffectChainPointer;

class EffectChain : public QObject {
    Q_OBJECT
  public:
    EffectChain(QObject* pParent, const unsigned int iChainNumber);
    virtual ~EffectChain();

    unsigned int numSlots() const;
    void addEffectSlot();

    void loadEffectToSlot(EffectPointer pEffect, unsigned int slotNumber);

  signals:
    void effectLoaded(EffectPointer pEffect, unsigned int slotNumber);

  private:
    DISALLOW_COPY_AND_ASSIGN(EffectChain);
    QString debugString() const {
        return QString("EffectChain(%1)").arg(m_iChainNumber);
    }

    mutable QMutex m_mutex;
    const unsigned int m_iChainNumber;
    QString m_group;
    QList<EffectSlotPointer> m_slots;
};


#endif /* EFFECTCHAIN_H */
