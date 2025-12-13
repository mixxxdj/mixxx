#pragma once

#include <QList>
#include <QObject>
#include <memory>

#include "effects/defs.h"
#include "effects/effectchainmixmode.h"
#include "engine/channelhandle.h"
#include "util/class.h"

class ControlObject;
class ControlPushButton;
class ControlEncoder;
class EffectsManager;
class EngineEffectChain;

/// EffectChain is the main thread representation of an effect chain.
/// EffectChain owns the ControlObjects for the routing switches that assign
/// chains to process audio inputs (decks, microphones, auxiliary inputs,
/// main mix). EffectChain also owns the ControlObject for the superknob
/// which manipulates the metaknob of each EffectSlot in the chain.
///
/// EffectChains are created and destroyed by EffectsManager during Mixxx
/// startup and shutdown. EffectChain adds exactly one EngineEffectChain to
/// the audio engine which is removed from the engine in the EffectChain
/// destructor. This is in contrast to EffectSlot, which loads/unloads
/// EngineEffects at the user's request.
///
/// The state of an EffectChain can be saved to and loaded from an
/// EffectChainPreset, which can serialize/deserialize that state to/from XML.
class EffectChain : public QObject {
    Q_OBJECT
  public:
    EffectChain(const QString& group,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger,
            SignalProcessingStage stage = SignalProcessingStage::Postfader);
    virtual ~EffectChain();

    QString group() const;
    void resetToDefault();

    EffectSlotPointer getEffectSlot(unsigned int slotNumber);

    void registerInputChannel(const ChannelHandleAndGroup& handleGroup,
            const double initialValue = 0.0);
    /// Do not store this in EffectSlot! The enabled input channels are a property
    /// of the chain, not the effect.
    const QSet<ChannelHandleAndGroup>& getActiveChannels() const {
        return m_enabledInputChannels;
    }

    double getSuperParameter() const;
    void setSuperParameter(double value, bool force = false);

    EffectChainMixMode::Type mixMode() const;
    void setMixMode(EffectChainMixMode::Type mixMode);

    const QString& getGroup() const {
        return m_group;
    }

    const QString& presetName() const;

    // Get the human-readable description of the EffectChain
    QString description() const;
    void setDescription(const QString& description);

    const QList<EffectSlotPointer>& getEffectSlots() const {
        return m_effectSlots;
    }

    virtual void loadChainPreset(EffectChainPresetPointer pPreset);

    bool isEmpty();

    bool isEmptyPlaceholderPresetLoaded();

    void loadEmptyNamelessPreset();

  public slots:
    void slotControlClear(double value);

  signals:
    void chainPresetChanged(const QString& name);

  protected slots:
    void sendParameterUpdate();
    void slotControlChainSuperParameter(double v, bool force);

  protected:
    EffectSlotPointer addEffectSlot(const QString& group);

    virtual int numPresets() const;

    // Activates EffectChain processing for the provided channel.
    void enableForInputChannel(const ChannelHandleAndGroup& handleGroup);
    void disableForInputChannel(const ChannelHandleAndGroup& handleGroup);

    // Protected so QuickEffectChain can use the separate QuickEffect
    // chain preset list.
    QString m_presetName;
    virtual int presetIndex() const;
    virtual EffectChainPresetPointer presetAtIndex(int index) const;

    EffectsManager* m_pEffectsManager;
    EffectChainPresetManagerPointer m_pChainPresetManager;
    EffectsMessengerPointer m_pMessenger;
    std::unique_ptr<ControlObject> m_pControlChainMix;
    std::unique_ptr<ControlObject> m_pControlChainSuperParameter;
    std::unique_ptr<ControlObject> m_pControlNumChainPresets;
    QList<EffectSlotPointer> m_effectSlots;

  protected slots:
    void slotEffectChainPresetRenamed(const QString& oldName, const QString& newName);
    void slotPresetListUpdated();

  private slots:
    void slotControlLoadedChainPresetRequest(double value);
    void slotControlChainPresetSelector(double value);
    void slotControlNextChainPreset(double value);
    void slotControlPrevChainPreset(double value);
    void slotChannelStatusChanged(double value, const ChannelHandleAndGroup& handleGroup);

  private:
    QString debugString() const {
        return QString("EffectChain(%1)").arg(m_group);
    }

    void addToEngine();
    void removeFromEngine();

    const QString m_group;

    std::unique_ptr<ControlPushButton> m_pControlClear;
    std::unique_ptr<ControlObject> m_pControlNumEffectSlots;
    std::unique_ptr<ControlPushButton> m_pControlChainEnabled;
    std::unique_ptr<ControlPushButton> m_pControlChainMixMode;
    std::unique_ptr<ControlObject> m_pControlLoadedChainPreset;
    std::unique_ptr<ControlEncoder> m_pControlChainPresetSelector;
    std::unique_ptr<ControlPushButton> m_pControlNextChainPreset;
    std::unique_ptr<ControlPushButton> m_pControlPrevChainPreset;

    void setControlLoadedPresetIndex(int index);

    // These COs do not affect how the effects are processed;
    // they are defined here for skins and controller mappings to communicate
    // with each other. They cannot be defined in skins because they must be present
    // when both skins and mappings are loaded, otherwise the skin will
    // create a new CO with the same ConfigKey but actually be interacting with a different
    // object than the mapping.
    std::unique_ptr<ControlPushButton> m_pControlChainShowFocus;
    std::unique_ptr<ControlPushButton> m_pControlChainHasControllerFocus;
    std::unique_ptr<ControlPushButton> m_pControlChainShowParameters;
    std::unique_ptr<ControlPushButton> m_pControlChainFocusedEffect;

    SignalProcessingStage m_signalProcessingStage;
    QHash<ChannelHandleAndGroup, std::shared_ptr<ControlPushButton>> m_channelEnableButtons;
    QSet<ChannelHandleAndGroup> m_enabledInputChannels;
    EngineEffectChain* m_pEngineEffectChain;

    DISALLOW_COPY_AND_ASSIGN(EffectChain);
};
