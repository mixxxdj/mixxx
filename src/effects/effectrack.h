#ifndef EFFECTRACK_H
#define EFFECTRACK_H

#include <QObject>
#include <QString>
#include <QSharedPointer>

#include "controlobject.h"
#include "effects/effectchainslot.h"

class EffectChainManager;

class EffectRack;
typedef QSharedPointer<EffectRack> EffectRackPointer;

class EffectRack : public QObject {
    Q_OBJECT
  public:
    EffectRack(EffectChainManager* pChainManager,
               const unsigned int iRackNumber);
    virtual ~EffectRack();

    static QString formatGroupString(const unsigned int iRackNumber) {
        return QString("[EffectRack%1]").arg(iRackNumber);
    }

    void registerGroup(const QString& group);
    int numEffectChainSlots() const;
    EffectChainSlotPointer addEffectChainSlot();
    EffectChainSlotPointer getEffectChainSlot(int i);

  public slots:
    void slotClearRack(double v);

  private slots:
    void loadNextChain(const unsigned int iChainSlotNumber,
                       EffectChainPointer pLoadedChain);
    void loadPrevChain(const unsigned int iChainSlotNumber,
                       EffectChainPointer pLoadedChain);

  private:
    const unsigned int m_iRackNumber;
    const QString m_group;

    EffectChainManager* m_pEffectChainManager;
    QList<EffectChainSlotPointer> m_effectChainSlots;
    ControlObject m_controlClearRack;
};


#endif /* EFFECTRACK_H */
