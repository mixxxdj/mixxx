#pragma once

#include <QHash>
#include <QList>
#include <QObject>
#include <QPair>
#include <QScopedPointer>
#include <QSet>

#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "effects/effectmanifestparameter.h"
#include "effects/backends/effectsbackendmanager.h"
#include "effects/presets/effectchainpresetmanager.h"
#include "effects/specialeffectchainslots.h"
#include "engine/channelhandle.h"
#include "engine/effects/engineeffectsmanager.h"
#include "preferences/usersettings.h"
#include "util/class.h"
#include "util/fifo.h"
#include "util/memory.h"
#include "util/xml.h"

class EngineEffectsManager;
class EffectManifest;

typedef QMap<EffectParameterType, QList<EffectParameterPointer>> ParameterMap;

/// EffectsManager is the interface between the parts of the effects system in
/// the main thread and the rest of Mixxx. It creates/destroys a fixed
/// set of EffectChainSlots on Mixxx startup/shutdown. EffectManager uses
/// EffectBackends and EffectManifests to create EffectProcessors. The
/// EffectProcessors are sent down to the EffectChainSlots, then down to the
/// EffectSlots which use the EffectProcessors to create EngineEffects and add
/// them to the audio engine.
///
/// EffectsManager saves/loads EffectPresets in the "effects/defaults" folder in
/// the user settings folder to allow users to specify default states when each
/// effect is loaded.
///
/// To maintain clear separation of responsibilities, GUI classes should NOT
/// access the EffectChainSlots or EffectSlots directly. They should interface
/// with them indirectly through EffectsManager.
class EffectsManager : public QObject {
    Q_OBJECT
  public:
    EffectsManager(QObject* pParent, UserSettingsPointer pConfig,
                   ChannelHandleFactory* pChannelHandleFactory);
    virtual ~EffectsManager();

    EngineEffectsManager* getEngineEffectsManager() {
        return m_pEngineEffectsManager;
    }

    const ChannelHandle getMasterHandle() {
        return m_pChannelHandleFactory->getOrCreateHandle("[Master]");
    }

    const EffectChainPresetManagerPointer getChainPresetManager() const {
        return m_pChainPresetManager;
    }

    const EffectPresetManagerPointer getEffectPresetManager() const {
        return m_pEffectPresetManager;
    }

    const EffectsBackendManagerPointer getBackendManager() const {
        return m_pBackendManager;
    }

    static const int kNumStandardEffectChains = 4;

    bool isAdoptMetaknobValueEnabled() const;

    void registerInputChannel(const ChannelHandleAndGroup& handle_group);
    const QSet<ChannelHandleAndGroup>& registeredInputChannels() const {
        return m_registeredInputChannels;
    }

    void registerOutputChannel(const ChannelHandleAndGroup& handle_group);
    const QSet<ChannelHandleAndGroup>& registeredOutputChannels() const {
        return m_registeredOutputChannels;
    }

    void loadStandardEffect(
            const int iChainSlotNumber,
            const int iEffectSlotNumber,
            const EffectManifestPointer pManifest);

    void loadOutputEffect(
            const int iEffectSlotNumber,
            const EffectManifestPointer pManifest);

    void loadEqualizerEffect(const QString& group,
            const int iEffectSlotNumber,
            const EffectManifestPointer pManifest);

    void loadEffect(
            EffectChainSlotPointer pChainSlot,
            const int iEffectSlotNumber,
            const EffectManifestPointer pManifest,
            EffectPresetPointer pPreset = nullptr,
            bool adoptMetaknobFromPreset = false);

    ParameterMap getLoadedParameters(int chainNumber, int effectNumber) const;
    ParameterMap getHiddenParameters(int chainNumber, int effectNumber) const;

    void hideParameter(int chainNumber, int effectNumber, EffectParameterPointer pParameter);
    void showParameter(int chainNumber, int effectNumber, EffectParameterPointer pParameter);

    void loadPresetToStandardChain(int chainNumber, EffectChainPresetPointer pPreset);

    void addStandardEffectChainSlots();
    EffectChainSlotPointer getStandardEffectChainSlot(int unitNumber) const;

    void addOutputEffectChainSlot();
    EffectChainSlotPointer getOutputEffectChainSlot() const;

    void addEqualizerEffectChainSlot(const QString& deckGroupName);
    void addQuickEffectChainSlot(const QString& deckGroupName);

    // TODO: Remove these methods to reduce coupling between GUI and
    // effects system implementation details.
    EffectChainSlotPointer getEffectChainSlot(const QString& group) const;
    EffectSlotPointer getEffectSlot(const QString& group);

    EffectParameterSlotBasePointer getEffectParameterSlot(
            const EffectParameterType parameterType, const ConfigKey& configKey);

    QString getNextEffectId(const QString& effectId);
    QString getPrevEffectId(const QString& effectId);

    inline const QList<EffectManifestPointer>& getVisibleEffectManifests() const {
        return m_visibleEffectManifests;
    };

    void setEffectVisibility(EffectManifestPointer pManifest, bool visibility);
    bool getEffectVisibility(EffectManifestPointer pManifest);

    void saveDefaultForEffect(EffectPresetPointer pEffectPreset);
    void saveDefaultForEffect(int chainNumber, int effcectNumber);

    void savePresetFromStandardEffectChain(int chainNumber);

    void setup();

  signals:
    void visibleEffectsUpdated();
    void effectChainPresetListUpdated();

  private:
    QString debugString() const {
        return "EffectsManager";
    }

    void connectChainSlotSignals(EffectChainSlotPointer pChainSlot);

    void loadDefaultEffectPresets();

    void readEffectsXml();
    void saveEffectsXml();

    ChannelHandleFactory* m_pChannelHandleFactory;

    QList<EffectManifestPointer> m_visibleEffectManifests;

    // We need to create Control Objects for Equalizers' frequencies
    ControlPotmeter m_loEqFreq;
    ControlPotmeter m_hiEqFreq;

    bool m_underDestruction;

    QSet<ChannelHandleAndGroup> m_registeredInputChannels;
    QSet<ChannelHandleAndGroup> m_registeredOutputChannels;
    UserSettingsPointer m_pConfig;
    QHash<QString, EffectChainSlotPointer> m_effectChainSlotsByGroup;

    QList<StandardEffectChainSlotPointer> m_standardEffectChainSlots;
    OutputEffectChainSlotPointer m_outputEffectChainSlot;
    QHash<QString, EqualizerEffectChainSlotPointer> m_equalizerEffectChainSlots;
    QHash<QString, QuickEffectChainSlotPointer> m_quickEffectChainSlots;

    EffectChainPresetPointer m_defaultQuickEffectChainPreset;

    EffectsBackendManagerPointer m_pBackendManager;
    EngineEffectsManager* m_pEngineEffectsManager;
    EffectsMessengerPointer m_pMessenger;
    EffectPresetManagerPointer m_pEffectPresetManager;
    EffectChainPresetManagerPointer m_pChainPresetManager;

    DISALLOW_COPY_AND_ASSIGN(EffectsManager);
};
