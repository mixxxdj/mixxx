#ifndef EFFECTCHAINMANAGER_H
#define EFFECTCHAINMANAGER_H

#include <QObject>
#include <QList>

#include "util.h"
#include "effects/effectchain.h"

class EffectsManager;

class EffectChainManager : public QObject {
    Q_OBJECT
  public:
    EffectChainManager(EffectsManager* pEffectsManager);
    virtual ~EffectChainManager();

    void addEffectChain(EffectChainPointer pEffectChain);

    EffectChainPointer getNextEffectChain(EffectChainPointer pEffectChain);
    EffectChainPointer getPrevEffectChain(EffectChainPointer pEffectChain);

    void saveEffectChains();
    void loadEffectChains();

  private:
    QString debugString() const {
        return "EffectChainManager";
    }

    EffectsManager* m_pEffectsManager;
    QList<EffectChainPointer> m_effectChains;

    DISALLOW_COPY_AND_ASSIGN(EffectChainManager);
};


#endif /* EFFECTCHAINMANAGER_H */
