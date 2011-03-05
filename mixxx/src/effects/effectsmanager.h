#ifndef EFFECTSMANAGER_H
#define EFFECTSMANAGER_H

#include <QObject>
#include <QMutex>
#include <QList>

#include "util.h"
#include "effects/effect.h"
#include "effects/effectsbackend.h"
#include "effects/effectchain.h"

class EffectsManager : public QObject {
    Q_OBJECT
  public:
    EffectsManager(QObject* pParent);
    virtual ~EffectsManager();

    // Add an effect backend to be managed by EffectsManager. EffectsManager
    // takes ownership of the backend, and will delete it when EffectsManager is
    // being deleted. Not thread safe -- use only from the GUI thread.
    void addEffectsBackend(EffectsBackend* pEffectsBackend);

    unsigned int numEffectChains() const;
    void addEffectChain();
    EffectChainPointer getEffectChain(unsigned int i);

  private:
    mutable QMutex m_mutex;
    QList<EffectsBackend*> m_effectsBackends;
    QList<EffectChainPointer> m_effectChains;

    DISALLOW_COPY_AND_ASSIGN(EffectsManager);
};


#endif /* EFFECTSMANAGER_H */
