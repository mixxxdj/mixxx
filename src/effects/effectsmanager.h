#ifndef EFFECTSMANAGER_H
#define EFFECTSMANAGER_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QSet>
#include <QScopedPointer>
#include <QPair>

#include "configobject.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"
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
    typedef bool (*EffectManifestFilterFnc)(EffectManifest* pManifest);

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

    StandardEffectRackPointer addStandardEffectRack();
    StandardEffectRackPointer getStandardEffectRack(int rack);

    EqualizerRackPointer addEqualizerRack();
    EqualizerRackPointer getEqualizerRack(int rack);

    QuickEffectRackPointer addQuickEffectRack();
    QuickEffectRackPointer getQuickEffectRack(int rack);

    EffectRackPointer getEffectRack(const QString& group);

    QString getNextEffectId(const QString& effectId);
    QString getPrevEffectId(const QString& effectId);

    const QList<QString> getAvailableEffects() const;
    // Each entry of the set is a pair containing the effect id and its name
    const QList<QPair<QString, QString> > getEffectNamesFiltered(EffectManifestFilterFnc filter) const;
    bool isEQ(const QString& effectId) const;
    QPair<EffectManifest, EffectsBackend*> getEffectManifestAndBackend(
            const QString& effectId) const;
    EffectManifest getEffectManifest(const QString& effectId) const;
    EffectPointer instantiateEffect(const QString& effectId);

    // Temporary, but for setting up all the default EffectChains and EffectRacks
    void setupDefaults();

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

    // We need to create Control Objects for Equalizers' frequencies
    ControlPotmeter* m_pLoEqFreq;
    ControlPotmeter* m_pHiEqFreq;

    DISALLOW_COPY_AND_ASSIGN(EffectsManager);
};


#endif /* EFFECTSMANAGER_H */
