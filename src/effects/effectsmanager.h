#pragma once

#include <QObject>
#include <QHash>
#include <QList>
#include <QSet>
#include <QScopedPointer>
#include <QPair>

#include "preferences/usersettings.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "engine/channelhandle.h"
#include "engine/effects/message.h"
#include "util/class.h"
#include "util/fifo.h"

class EngineEffectsManager;
class EffectChainManager;
class EffectManifest;
class EffectsBackend;

class EffectsManager : public QObject {
    Q_OBJECT
  public:
    typedef bool (*EffectManifestFilterFnc)(EffectManifest* pManifest);

    EffectsManager(QObject* pParent,
            UserSettingsPointer pConfig,
            ChannelHandleFactoryPointer pChannelHandleFactory);
    virtual ~EffectsManager();

    EngineEffectsManager* getEngineEffectsManager() {
        return m_pEngineEffectsManager;
    }

    EffectChainManager* getEffectChainManager() {
        return m_pEffectChainManager;
    }

    const ChannelHandle getMasterHandle() {
        return m_pChannelHandleFactory->getOrCreateHandle("[Master]");
    }

    // Add an effect backend to be managed by EffectsManager. EffectsManager
    // takes ownership of the backend, and will delete it when EffectsManager is
    // being deleted. Not thread safe -- use only from the GUI thread.
    void addEffectsBackend(EffectsBackend* pEffectsBackend);
    void registerInputChannel(const ChannelHandleAndGroup& handle_group);
    void registerOutputChannel(const ChannelHandleAndGroup& handle_group);
    const QSet<ChannelHandleAndGroup>& registeredInputChannels() const;
    const QSet<ChannelHandleAndGroup>& registeredOutputChannels() const;

    StandardEffectRackPointer addStandardEffectRack();
    StandardEffectRackPointer getStandardEffectRack(int rack);

    EqualizerRackPointer addEqualizerRack();
    EqualizerRackPointer getEqualizerRack(int rack);

    QuickEffectRackPointer addQuickEffectRack();
    QuickEffectRackPointer getQuickEffectRack(int rack);

    OutputEffectRackPointer addOutputsEffectRack();
    OutputEffectRackPointer getOutputsEffectRack();

    void loadEffectChains();

    EffectRackPointer getEffectRack(const QString& group);
    EffectSlotPointer getEffectSlot(const QString& group);

    EffectParameterSlotPointer getEffectParameterSlot(
            const ConfigKey& configKey);
    EffectButtonParameterSlotPointer getEffectButtonParameterSlot(
            const ConfigKey& configKey);

    QString getNextEffectId(const QString& effectId);
    QString getPrevEffectId(const QString& effectId);

    inline const QList<EffectManifestPointer>& getAvailableEffectManifests() const {
        return m_availableEffectManifests;
    };
    inline const QList<EffectManifestPointer>& getVisibleEffectManifests() const {
        return m_visibleEffectManifests;
    };
    const QList<EffectManifestPointer> getAvailableEffectManifestsFiltered(
        EffectManifestFilterFnc filter) const;
    bool isEQ(const QString& effectId) const;
    void getEffectManifestAndBackend(
            const QString& effectId,
            EffectManifestPointer* ppManifest, EffectsBackend** ppBackend) const;
    EffectManifestPointer getEffectManifest(const QString& effectId) const;
    EffectPointer instantiateEffect(const QString& effectId);

    void setEffectVisibility(EffectManifestPointer pManifest, bool visibility);
    bool getEffectVisibility(EffectManifestPointer pManifest);

    // Temporary, but for setting up all the default EffectChains and EffectRacks
    void setup();

    // Reloads all effect to the slots to update parameter assignments
    void refeshAllRacks();

    // Write an EffectsRequest to the EngineEffectsManager. EffectsManager takes
    // ownership of request and deletes it once a response is received.
    bool writeRequest(EffectsRequest* request);

  signals:
    // TODO() Not connected. Can be used when we implement effect PlugIn loading at runtime
    void availableEffectsUpdated(EffectManifestPointer);
    void visibleEffectsUpdated();

  private slots:
    void slotBackendRegisteredEffect(EffectManifestPointer pManifest);

  private:
    QString debugString() const {
        return "EffectsManager";
    }

    void processEffectsResponses();
    void collectGarbage(const EffectsRequest* pResponse);

    ChannelHandleFactoryPointer m_pChannelHandleFactory;

    EffectChainManager* m_pEffectChainManager;
    QList<EffectsBackend*> m_effectsBackends;
    QList<EffectManifestPointer> m_availableEffectManifests;
    QList<EffectManifestPointer> m_visibleEffectManifests;

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
