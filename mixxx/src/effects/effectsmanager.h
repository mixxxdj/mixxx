#ifndef EFFECTSMANAGER_H
#define EFFECTSMANAGER_H

#include "util.h"
#include "effects/effect.h"
#include "effects/effectsbackend.h"

class EffectsManager : public QObject {
    Q_OBJECT
  public:
    EffectsManager(QObject* pParent);
    virtual ~EffectsManager();

    // Add an effect backend to be managed by EffectsManager. EffectsManager
    // takes ownership of the backend, and will delete it when EffectsManager is
    // being deleted. Not thread safe -- use only from the GUI thread.
    void addEffectsBackend(EffectsBackend* pEffectsBackend);

    unsigned int numEffectSlots() const;
    void addEffectSlot();
    EffectSlot& getEffectSlot(unsigned int i);

    unsigned int numEffectChains() const;
    void addEffectChain();
    EffectChain& getEffectChain(unsigned int i);

  private:
    DISALLOW_COPY_AND_ASSIGN(EffectsManager);

    QList<EffectsBackend*> m_effectsBackends;
};


#endif /* EFFECTSMANAGER_H */
