#pragma once

#include <QHash>
#include <QList>
#include <QSet>

#include "control/controlpotmeter.h"
#include "effects/backends/effectsbackendmanager.h"
#include "effects/presets/effectchainpresetmanager.h"
#include "effects/specialeffectchainslots.h"
#include "engine/channelhandle.h"
#include "preferences/usersettings.h"
#include "util/class.h"

class EngineEffectsManager;

/// EffectsManager initializes and shuts down the effects system. It creates and
/// destroys a fixed set of StandardEffectChainSlots on Mixxx startup/shutdown
/// and creates a QuickEffectChainSlot and EqualizerEffectChainSlot when
/// PlayerManager creates decks.
class EffectsManager {
  public:
    EffectsManager(UserSettingsPointer pConfig,
            ChannelHandleFactory* pChannelHandleFactory);
    virtual ~EffectsManager();

    void setup();
    void addDeck(const QString& deckGroupName);

    EffectChainSlotPointer getEffectChainSlot(const QString& group) const;
    EqualizerEffectChainSlotPointer getEqualizerEffectChainSlot(
            const QString& deckGroupName) const {
        return m_equalizerEffectChainSlots.value(deckGroupName);
    }
    EffectChainSlotPointer getStandardEffectChainSlot(int unitNumber) const;
    EffectChainSlotPointer getOutputEffectChainSlot() const;

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

    bool isAdoptMetaknobValueEnabled() const;

  private:
    void addStandardEffectChainSlots();
    void addOutputEffectChainSlot();

    void addEqualizerEffectChainSlot(const QString& deckGroupName);
    void addQuickEffectChainSlot(const QString& deckGroupName);

    void readEffectsXml();
    void saveEffectsXml();

    QSet<ChannelHandleAndGroup> m_registeredInputChannels;
    QSet<ChannelHandleAndGroup> m_registeredOutputChannels;
    UserSettingsPointer m_pConfig;
    QHash<QString, EffectChainSlotPointer> m_effectChainSlotsByGroup;

    QList<StandardEffectChainSlotPointer> m_standardEffectChainSlots;
    OutputEffectChainSlotPointer m_outputEffectChainSlot;
    QHash<QString, EqualizerEffectChainSlotPointer> m_equalizerEffectChainSlots;
    QHash<QString, QuickEffectChainSlotPointer> m_quickEffectChainSlots;

    EffectsBackendManagerPointer m_pBackendManager;
    ChannelHandleFactory* m_pChannelHandleFactory;
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
