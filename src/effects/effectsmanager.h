#ifndef EFFECTSMANAGER_H
#define EFFECTSMANAGER_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QSet>
#include <QScopedPointer>

#include "configobject.h"
#include "util.h"
#include "util/fifo.h"
#include "effects/effect.h"
#include "effects/effectsbackend.h"
#include "effects/effectchainslot.h"
#include "effects/effectchain.h"
#include "effects/effectchainmanager.h"
#include "effects/effectrack.h"
#include "engine/effects/message.h"

class EngineEffectsManager;

class EffectsManager : public QObject {
    Q_OBJECT
  public:
    EffectsManager(QObject* pParent, ConfigObject<ConfigValue>* pConfig);
    virtual ~EffectsManager();

    EngineEffectsManager* getEngineEffectsManager() {
        return m_pEngineEffectsManager;
    }

    EffectChainManager* getEffectChainManager() {
        return m_pEffectChainManager;
    }

    // Add an effect backend to be managed by EffectsManager. EffectsManager
    // takes ownership of the backend, and will delete it when EffectsManager is
    // being deleted. Not thread safe -- use only from the GUI thread.
    void addEffectsBackend(EffectsBackend* pEffectsBackend);
    void registerGroup(const QString& group);
    const QSet<QString>& registeredGroups() const;

    EffectRackPointer addEffectRack();
    EffectRackPointer getEffectRack(int rack);

    QString getNextEffectId(const QString& effectId);
    QString getPrevEffectId(const QString& effectId);

    const QSet<QString> getAvailableEffects() const;
    const QSet<QString> getAvailableEffectNames() const;
    const QSet<QString> getAvailableEQEffects() const;
    const QSet<QString> getAvailableEQEffectNames() const;
    EffectManifest getEffectManifest(const QString& effectId) const;
    EffectPointer instantiateEffect(const QString& effectId);

    // Temporary, but for setting up all the default EffectChains and EffectRacks
    void setupDefaults();

    // Add an Equalizer for the specified channel
    void addEqualizer(int channelNumber);

    // Write an EffectsRequest to the EngineEffectsManager. EffectsManager takes
    // ownership of request and deletes it once a response is received.
    bool writeRequest(EffectsRequest* request);

  signals:
    void availableEffectsUpdated();

  private:
    QString debugString() const {
        return "EffectsManager";
    }

    void processEffectsResponses();

    EffectChainManager* m_pEffectChainManager;
    QList<EffectsBackend*> m_effectsBackends;

    EngineEffectsManager* m_pEngineEffectsManager;

    QScopedPointer<EffectsRequestPipe> m_pRequestPipe;
    qint64 m_nextRequestId;
    QHash<qint64, EffectsRequest*> m_activeRequests;

    DISALLOW_COPY_AND_ASSIGN(EffectsManager);
};


#endif /* EFFECTSMANAGER_H */
