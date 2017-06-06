#ifndef EFFECTSMANAGER_H
#define EFFECTSMANAGER_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QSet>
#include <QScopedPointer>
#include <QPair>

#include "preferences/usersettings.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "effects/effect.h"
#include "effects/effectchain.h"
#include "effects/effectchainmanager.h"
#include "effects/effectchainslot.h"
#include "effects/effectrack.h"
#include "effects/effectsbackend.h"
#include "engine/channelhandle.h"
#include "engine/effects/message.h"
#include "util/class.h"
#include "util/fifo.h"

class EngineEffectsManager;

class EffectsManager : public QObject {
    Q_OBJECT
  public:
    typedef bool (*EffectManifestFilterFnc)(const EffectManifest& pManifest);

    EffectsManager(QObject* pParent, UserSettingsPointer pConfig);
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
    void registerChannel(const ChannelHandleAndGroup& handle_group);
    const QSet<ChannelHandleAndGroup>& registeredChannels() const;

    StandardEffectRackPointer addStandardEffectRack();
    StandardEffectRackPointer getStandardEffectRack(int rack);

    EqualizerRackPointer addEqualizerRack();
    EqualizerRackPointer getEqualizerRack(int rack);

    QuickEffectRackPointer addQuickEffectRack();
    QuickEffectRackPointer getQuickEffectRack(int rack);

    EffectRackPointer getEffectRack(const QString& group);

    QString getNextEffectId(const QString& effectId);
    QString getPrevEffectId(const QString& effectId);

    inline const QList<EffectManifest>& getAvailableEffectManifests() const {
        return m_availableEffectManifests;
    };
    const QList<EffectManifest> getAvailableEffectManifestsFiltered(
        EffectManifestFilterFnc filter) const;
    bool isEQ(const QString& effectId) const;
    QPair<EffectManifest, EffectsBackend*> getEffectManifestAndBackend(
            const QString& effectId) const;
    EffectManifest getEffectManifest(const QString& effectId) const;
    EffectPointer instantiateEffect(const QString& effectId);

    // Temporary, but for setting up all the default EffectChains and EffectRacks
    void setup();

    // Write an EffectsRequest to the EngineEffectsManager. EffectsManager takes
    // ownership of request and deletes it once a response is received.
    bool writeRequest(EffectsRequest* request);

  signals:
    void availableEffectsUpdated(EffectManifest);

  private slots:
    void slotBackendRegisteredEffect(EffectManifest manifest);

  private:
    QString debugString() const {
        return "EffectsManager";
    }

    void processEffectsResponses();

    EffectChainManager* m_pEffectChainManager;
    QList<EffectsBackend*> m_effectsBackends;
    QList<EffectManifest> m_availableEffectManifests;

    EngineEffectsManager* m_pEngineEffectsManager;

    QScopedPointer<EffectsRequestPipe> m_pRequestPipe;
    qint64 m_nextRequestId;
    QHash<qint64, EffectsRequest*> m_activeRequests;

    ControlObject* m_pNumEffectsAvailable;
    // We need to create Control Objects for Equalizers' frequencies
    ControlPotmeter* m_pLoEqFreq;
    ControlPotmeter* m_pHiEqFreq;

    bool m_underDestruction;

    DISALLOW_COPY_AND_ASSIGN(EffectsManager);
};


#endif /* EFFECTSMANAGER_H */
