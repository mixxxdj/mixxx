#ifndef EFFECTRACK_H
#define EFFECTRACK_H

#include <QObject>
#include <QString>
#include <QSharedPointer>

#include "controlobject.h"
#include "effects/effectchainslot.h"

class EngineEffectRack;
class EffectsManager;
class EffectChainManager;

class EffectRack;
typedef QSharedPointer<EffectRack> EffectRackPointer;

class EffectRack : public QObject {
    Q_OBJECT
  public:
    EffectRack(EffectsManager* pEffectsManager,
               EffectChainManager* pChainManager,
               const unsigned int iRackNumber);
    virtual ~EffectRack();

    static QString formatGroupString(const unsigned int iRackNumber) {
        return QString("[EffectRack%1]").arg(iRackNumber);
    }

    void addToEngine();
    void removeFromEngine();
    EngineEffectRack* getEngineEffectRack();

    void registerGroup(const QString& group);
    int numEffectChainSlots() const;
    EffectChainSlotPointer addEffectChainSlot();
    EffectChainSlotPointer getEffectChainSlot(int i);

  public slots:
    void slotClearRack(double v);
    void slotNumEffectChainSlots(double v);

  private slots:
    void loadNextChain(const unsigned int iChainSlotNumber,
                       EffectChainPointer pLoadedChain);
    void loadPrevChain(const unsigned int iChainSlotNumber,
                       EffectChainPointer pLoadedChain);

    void loadNextEffect(const unsigned int iChainSlotNumber,
                        const unsigned int iEffectSlotNumber,
                        EffectPointer pEffect);
    void loadPrevEffect(const unsigned int iChainSlotNumber,
                        const unsigned int iEffectSlotNumber,
                        EffectPointer pEffect);

  private:
    inline QString debugString() const {
        return QString("EffectRack%1").arg(m_iRackNumber);
    }

    const unsigned int m_iRackNumber;
    const QString m_group;

    EffectsManager* m_pEffectsManager;
    EffectChainManager* m_pEffectChainManager;
    QList<EffectChainSlotPointer> m_effectChainSlots;
    ControlObject m_controlNumEffectChainSlots;
    ControlObject m_controlClearRack;
    EngineEffectRack* m_pEngineEffectRack;
};


#endif /* EFFECTRACK_H */
