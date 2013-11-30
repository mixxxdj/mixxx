#ifndef EFFECTSMANAGER_H
#define EFFECTSMANAGER_H

#include <QObject>
#include <QMutex>
#include <QHash>
#include <QList>
#include <QSet>
#include <QScopedPointer>

#include "util.h"
#include "util/fifo.h"
#include "effects/effect.h"
#include "effects/effectsbackend.h"
#include "effects/effectchainslot.h"
#include "effects/effectchain.h"
#include "engine/effects/message.h"

class EngineEffectsManager;
class EffectChainManager;

class EffectsManager : public QObject {
    Q_OBJECT
  public:
    EffectsManager(QObject* pParent);
    virtual ~EffectsManager();

    EngineEffectsManager* getEngineEffectsManager() {
        return m_pEngineEffectsManager;
    }

    // Add an effect backend to be managed by EffectsManager. EffectsManager
    // takes ownership of the backend, and will delete it when EffectsManager is
    // being deleted. Not thread safe -- use only from the GUI thread.
    void addEffectsBackend(EffectsBackend* pEffectsBackend);

    unsigned int numEffectChainSlots() const;
    void addEffectChainSlot();
    EffectChainSlotPointer getEffectChainSlot(unsigned int i);

    void registerGroup(const QString& group);

    const QSet<QString> getAvailableEffects() const;
    EffectManifest getEffectManifest(const QString& effectId) const;
    EffectPointer instantiateEffect(const QString& effectId);

    // Temporary, but for setting up all the default EffectChains
    void setupDefaultChains();

    // Write an EffectsRequest to the EngineEffectsManager. EffectsManager takes
    // ownership of request and deletes it once a response is received.
    bool writeRequest(EffectsRequest* request);

  private slots:
    void loadNextChain(const unsigned int iChainSlotNumber, EffectChainPointer pLoadedChain);
    void loadPrevChain(const unsigned int iChainSlotNumber, EffectChainPointer pLoadedChain);

  private:
    QString debugString() const {
        return "EffectsManager";
    }

    void processEffectsResponses();

    mutable QMutex m_mutex;
    EffectChainManager* m_pEffectChainManager;
    QList<EffectsBackend*> m_effectsBackends;
    QList<EffectChainSlotPointer> m_effectChainSlots;
    QSet<QString> m_registeredGroups;

    EngineEffectsManager* m_pEngineEffectsManager;

    QScopedPointer<EffectsRequestPipe> m_pRequestPipe;
    qint64 m_nextRequestId;
    QHash<qint64, EffectsRequest*> m_activeRequests;

    DISALLOW_COPY_AND_ASSIGN(EffectsManager);
};


#endif /* EFFECTSMANAGER_H */
