#ifndef EFFECTCHAINMANAGER_H
#define EFFECTCHAINMANAGER_H

#include <QObject>
#include <QList>

#include "util.h"
#include "effects/effectchain.h"
#include "effects/effectrack.h"

class EffectsManager;

// A class for keeping track of all the user's EffectChains. Eventually will
// serialize/deserialize the EffectChains from storage but for Effects v1 we are
// hard-coding the available chains.
class EffectChainManager : public QObject {
    Q_OBJECT
  public:
    EffectChainManager(EffectsManager* pEffectsManager);
    virtual ~EffectChainManager();

    void registerGroup(const QString& group);
    const QSet<QString>& registeredGroups() const {
        return m_registeredGroups;
    }

    EffectRackPointer addEffectRack();
    EffectRackPointer getEffectRack(int i);

    void addEffectChain(EffectChainPointer pEffectChain);
    void removeEffectChain(EffectChainPointer pEffectChain);

    // To support cycling through effect chains, there is a global ordering of
    // chains. These methods allow you to get the next or previous chain given
    // your current chain.
    // TODO(rryan): Prevent double-loading of a chain into a slot?
    EffectChainPointer getNextEffectChain(EffectChainPointer pEffectChain);
    EffectChainPointer getPrevEffectChain(EffectChainPointer pEffectChain);

    void saveEffectChains();
    void loadEffectChains();

  private:
    QString debugString() const {
        return "EffectChainManager";
    }

    EffectsManager* m_pEffectsManager;
    QList<EffectRackPointer> m_effectRacks;
    QList<EffectChainPointer> m_effectChains;
    QSet<QString> m_registeredGroups;
    DISALLOW_COPY_AND_ASSIGN(EffectChainManager);
};

#endif /* EFFECTCHAINMANAGER_H */
