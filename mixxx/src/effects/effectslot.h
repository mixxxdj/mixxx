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
    // Request that this EffectSlot unload the currently loaded effect
    void clear();

  signals:
    void effectLoaded(EffectPointer pEffect);
    void effectUnloaded(EffectPointer pEffect);

  private:
    DISALLOW_COPY_AND_ASSIGN(EffectSlot);
    QString debugString() const {
        return QString("EffectChain(%1)").arg(m_iChainNumber);
    }

    mutable QMutex m_mutex;
    const unsigned int m_iChainNumber;
    const unsigned int m_iSlotNumber;
    QString m_group;
    EffectPointer m_pEffect;

    ControlObject* m_pControlEnabled;
    ControlObject* m_pControlNumParameters;
    QList<EffectParameterSlot*> m_parameters;
};

#endif /* EFFECTSLOT_H */
