#pragma once

#include <QDomDocument>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSignalMapper>

#include "control/controlobject.h"
#include "effects/defs.h"
#include "effects/presets/effectchainpreset.h"
#include "engine/channelhandle.h"
#include "util/class.h"
#include "util/memory.h"

class ControlPushButton;
class ControlEncoder;
class EffectChainSlot;
class EffectsManager;
class EffectProcessor;
class EngineEffectChain;

// EffectChainSlot is the main thread representation of an effect chain which
// adds/removes exactly one EngineEffectChain from the engine. Unlike EffectSlot,
// EffectChainSlot does not add/remove EngineEffectChains apart from Mixxx
// startup and shutdown. The lifetime of EngineEffectChain is coupled with
// EffectChainSlot. Do not change this relationship; there is no use case for
// that.

// EffectChainSlot owns the ControlObjects for the routing switches that assign
// chains to process audio inputs (decks, microphones, auxiliary inputs,
// master mix). EffectChainSlot also owns the ControlObject for the superknob
// which manipulates the metaknob of each effect in the chain.

// The state of an EffectChainSlot can be saved to and loaded from an
// EffectChainPreset, which can serialize/deserialize that state to/from XML.
// Loading state from an EffectChainPreset is done by
// EffectsManager::loadEffectChainPreset rather than directly by EffectChainSlot
// because loading effects requires access to the EffectsBackends and default
// EffectPresets which are maintained by EffectsManager.

// Currently EffectChainSlot has a fixed number of EffectSlots. In the future
// this may be extended to create an Effect class to decouple an EngineEffect's
// state from the ControlObjects so that EffectChainSlot could arbitrarily hide
// and rearrange Effects by loading/unloading them from EffectSlots. This would
// be similar to the relationship between EffectSlot and
// EffectParameterSlotBase/EffectParameter.
class EffectChainSlot : public QObject {
    Q_OBJECT
  public:
    EffectChainSlot(const QString& group,
                    EffectsManager* pEffectsManager,
                    SignalProcessingStage stage = SignalProcessingStage::Postfader,
                    const QString& id = QString());
    virtual ~EffectChainSlot();

    // Get the ID of the loaded EffectChain
    QString id() const;
    QString group() const;

    EffectSlotPointer getEffectSlot(unsigned int slotNumber);

    void registerInputChannel(const ChannelHandleAndGroup& handle_group,
                              const double initialValue = 0.0);
    QSet<ChannelHandleAndGroup> getActiveChannels() const {
        return m_enabledInputChannels;
    }

    double getSuperParameter() const;
    void setSuperParameter(double value, bool force = false);
    void setSuperParameterDefaultValue(double value);

    EffectChainMixMode mixMode() const {
        return m_mixMode;
    }
    void setMixMode(EffectChainMixMode mixMode);

    const QString& getGroup() const {
        return m_group;
    }

    // Get the human-readable name of the EffectChain
    const QString& name() const;
    void setName(const QString& name);

    // Get the human-readable description of the EffectChain
    QString description() const;
    void setDescription(const QString& description);

    static QString mixModeToString(EffectChainMixMode type) {
        switch (type) {
            case EffectChainMixMode::DrySlashWet:
                return "DRY/WET";
            case EffectChainMixMode::DryPlusWet:
                return "DRY+WET";
            default:
                return "UNKNOWN";
        }
    }
    static EffectChainMixMode mixModeFromString(const QString& typeStr) {
        if (typeStr == "DRY/WET") {
            return EffectChainMixMode::DrySlashWet;
        } else if (typeStr == "DRY+WET") {
            return EffectChainMixMode::DryPlusWet;
        } else {
            return EffectChainMixMode::NumMixModes;
        }
    }

    const QList<EffectSlotPointer>& getEffectSlots() const {
        return m_effectSlots;
    }

    virtual void loadEffect(const unsigned int iEffectSlotNumber,
            const EffectManifestPointer pManifest,
            std::unique_ptr<EffectProcessor> pProcessor,
            EffectPresetPointer pPreset,
            bool adoptMetaknobFromPreset = false);

  signals:
    // Signal that whoever is in charge of this EffectChainSlot should load the
    // next EffectChain into it.
    void nextChain(unsigned int iChainSlotNumber,
                   EffectChainSlotPointer pEffectChain);

    // Signal that whoever is in charge of this EffectChainSlot should load the
    // previous EffectChain into it.
    void prevChain(unsigned int iChainSlotNumber,
                   EffectChainSlotPointer pEffectChain);

    // Signal that whoever is in charge of this EffectChainSlot should clear
    // this EffectChain (by removing the chain from this EffectChainSlot).
    void clearChain(unsigned int iChainNumber, EffectChainSlotPointer pEffectChain);

    // Signal that indicates that the EffectChainSlot has been updated.
    void updated();

  protected slots:
    void sendParameterUpdate();
    void slotControlChainSuperParameter(double v, bool force = false);

  protected:
    EffectSlotPointer addEffectSlot(const QString& group);

    // Activates EffectChain processing for the provided channel.
    void enableForInputChannel(const ChannelHandleAndGroup& handle_group);
    void disableForInputChannel(const ChannelHandleAndGroup& handle_group);

    EffectsManager* m_pEffectsManager;
    ControlObject* m_pControlChainMix;
    ControlObject* m_pControlChainSuperParameter;
    QList<EffectSlotPointer> m_effectSlots;

  private slots:
    void slotControlClear(double v);
    void slotControlChainSelector(double v);
    void slotControlChainNextPreset(double v);
    void slotControlChainPrevPreset(double v);
    void slotChannelStatusChanged(const QString& group);

  private:
    QString debugString() const {
        return QString("EffectChainSlot(%1)").arg(m_group);
    }

    void addToEngine();
    void removeFromEngine();

    const QString m_group;

    ControlPushButton* m_pControlClear;
    ControlObject* m_pControlNumEffects;
    ControlObject* m_pControlNumEffectSlots;
    ControlObject* m_pControlChainLoaded;
    ControlPushButton* m_pControlChainEnabled;
    ControlPushButton* m_pControlChainMixMode;
    ControlEncoder* m_pControlChainSelector;
    ControlPushButton* m_pControlChainNextPreset;
    ControlPushButton* m_pControlChainPrevPreset;

    /**
      These COs do not affect how the effects are processed;
      they are defined here for skins and controller mappings to communicate
      with each other. They cannot be defined in skins because they must be present
      when both skins and mappings are loaded, otherwise the skin will
      create a new CO with the same ConfigKey but actually be interacting with a different
      object than the mapping.
    **/
    ControlPushButton* m_pControlChainShowFocus;
    ControlPushButton* m_pControlChainShowParameters;
    ControlPushButton* m_pControlChainFocusedEffect;

    struct ChannelInfo {
        // Takes ownership of pEnabled.
        ChannelInfo(const ChannelHandleAndGroup& handle_group, ControlObject* pEnabled)
                : handle_group(handle_group),
                  pEnabled(pEnabled) {

        }
        ~ChannelInfo() {
            delete pEnabled;
        }
        ChannelHandleAndGroup handle_group;
        ControlObject* pEnabled;
    };

    QMap<QString, ChannelInfo*> m_channelInfoByName;
    QSignalMapper m_channelStatusMapper;
    QString m_id;
    QString m_name;
    QString m_description;
    EffectChainMixMode m_mixMode;
    SignalProcessingStage m_signalProcessingStage;
    QSet<ChannelHandleAndGroup> m_enabledInputChannels;
    EngineEffectChain* m_pEngineEffectChain;

    DISALLOW_COPY_AND_ASSIGN(EffectChainSlot);
};
