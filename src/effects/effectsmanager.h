#pragma once

#include <QHash>
#include <QList>
#include <QSet>

#include "control/controlpotmeter.h"
#include "effects/backends/effectsbackendmanager.h"
#include "effects/presets/effectchainpresetmanager.h"
#include "engine/channelhandle.h"
#include "preferences/usersettings.h"
#include "util/class.h"

class EngineEffectsManager;

/// EffectsManager initializes and shuts down the effects system. It creates and
/// destroys a fixed set of StandardEffectChains on Mixxx startup/shutdown
/// and creates a QuickEffectChain and EqualizerEffectChain when
/// PlayerManager creates decks. It also initializes a handful of sub-manager classes
/// responsible for specific parts of the effects system.
class EffectsManager {
  public:
    EffectsManager(UserSettingsPointer pConfig,
            std::shared_ptr<ChannelHandleFactory> pChannelHandleFactory);

    virtual ~EffectsManager();

    void setup();
    void addDeck(const QString& deckGroupName);

    EffectChainPointer getEffectChain(const QString& group) const;
    EqualizerEffectChainPointer getEqualizerEffectChain(
            const QString& deckGroupName) const {
        return m_equalizerEffectChains.value(deckGroupName);
    }
    EffectChainPointer getStandardEffectChain(int unitNumber) const;
    EffectChainPointer getOutputEffectChain() const;

    EngineEffectsManager* getEngineEffectsManager() const {
        return m_pEngineEffectsManager;
    }

    const ChannelHandle getMasterHandle() const {
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

    const VisibleEffectsListPointer getVisibleEffectsList() const {
        return m_pVisibleEffectsList;
    }

    void registerInputChannel(const ChannelHandleAndGroup& handle_group);
    const QSet<ChannelHandleAndGroup>& registeredInputChannels() const {
        return m_registeredInputChannels;
    }

    void registerOutputChannel(const ChannelHandleAndGroup& handle_group);
    const QSet<ChannelHandleAndGroup>& registeredOutputChannels() const {
        return m_registeredOutputChannels;
    }

    bool isAdoptMetaknobSettingEnabled() const;

  private:
    void addStandardEffectChains();
    void addOutputEffectChain();

    void addEqualizerEffectChain(const QString& deckGroupName);
    void addQuickEffectChain(const QString& deckGroupName);

    void readEffectsXml();
    void saveEffectsXml();

    QSet<ChannelHandleAndGroup> m_registeredInputChannels;
    QSet<ChannelHandleAndGroup> m_registeredOutputChannels;
    UserSettingsPointer m_pConfig;
    QHash<QString, EffectChainPointer> m_effectChainSlotsByGroup;

    QList<StandardEffectChainPointer> m_standardEffectChains;
    OutputEffectChainPointer m_outputEffectChain;
    // These two store <deck group, effect chain pointer>
    QHash<QString, EqualizerEffectChainPointer> m_equalizerEffectChains;
    QHash<QString, QuickEffectChainPointer> m_quickEffectChains;

    EffectsBackendManagerPointer m_pBackendManager;
    std::shared_ptr<ChannelHandleFactory> m_pChannelHandleFactory;

    EngineEffectsManager* m_pEngineEffectsManager;
    EffectsMessengerPointer m_pMessenger;
    VisibleEffectsListPointer m_pVisibleEffectsList;
    EffectPresetManagerPointer m_pEffectPresetManager;
    EffectChainPresetManagerPointer m_pChainPresetManager;

    // ControlObjects for Equalizers' frequencies
    // TODO: replace these with effect parameters that are hidden by default
    ControlPotmeter m_loEqFreq;
    ControlPotmeter m_hiEqFreq;

    DISALLOW_COPY_AND_ASSIGN(EffectsManager);
};
