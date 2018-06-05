#ifndef EFFECTCHAINSLOT_H
#define EFFECTCHAINSLOT_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QSignalMapper>
#include <QDomDocument>

#include "effects/defs.h"
#include "effects/effect.h"
#include "engine/channelhandle.h"
#include "util/class.h"


class ControlObject;
class ControlPushButton;
class ControlEncoder;
class EffectChainSlot;

class EffectChainSlot : public QObject {
    Q_OBJECT
  public:
    EffectChainSlot(EffectRack* pRack,
                    const QString& group,
                    const unsigned int iChainNumber,
                    EffectsManager* pEffectsManager,
                    const QString& id = QString());
    virtual ~EffectChainSlot();

    // Get the ID of the loaded EffectChain
    QString id() const;

    EffectSlotPointer addEffectSlot(const QString& group);
    EffectSlotPointer getEffectSlot(unsigned int slotNumber);

    void registerInputChannel(const ChannelHandleAndGroup& handle_group);

    double getSuperParameter() const;
    void setSuperParameter(double value, bool force = false);
    void setSuperParameterDefaultValue(double value);

    // Unload the loaded EffectChain.
    void clear();

    unsigned int getChainSlotNumber() const;

    const QString& getGroup() const {
        return m_group;
    }

    QDomElement toXml(QDomDocument* doc) const;
    void loadChainSlotFromXml(const QDomElement& effectChainElement);


    // Activates EffectChain processing for the provided channel.
    // TODO(Kshitij) : Make this function private once EffectRack layer is removed
    void enableForInputChannel(const ChannelHandleAndGroup& handle_group);
    void disableForInputChannel(const ChannelHandleAndGroup& handle_group);

    // Get the human-readable name of the EffectChain
    const QString& name() const;
    void setName(const QString& name);

    // Get the human-readable description of the EffectChain
    QString description() const;
    void setDescription(const QString& description);

    // TODO(Kshitij) : Remove this setter function once the EffectRack layer is removed
    void setMix(const double& dMix);

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

    void addEffect(EffectPointer pEffect);
    void replaceEffect(unsigned int effectSlotNumber, EffectPointer pEffect);
    void removeEffect(unsigned int effectSlotNumber);
    void refreshAllEffects();

    const QList<EffectPointer>& effects() const;

  signals:
    // Indicates that the given EffectChain was loaded into this
    // EffectChainSlot
    void effectChainLoaded(EffectChainSlotPointer pEffectChain);

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

    // Signal that whoever is in charge of this EffectChainSlot should load the
    // next Effect into the specified EffectSlot.
    void nextEffect(unsigned int iChainSlotNumber,
                    unsigned int iEffectSlotNumber,
                    EffectPointer pEffect);

    // Signal that whoever is in charge of this EffectChainSlot should load the
    // previous Effect into the specified EffectSlot.
    void prevEffect(unsigned int iChainSlotNumber,
                    unsigned int iEffectSlotNumber,
                    EffectPointer pEffect);

    // Signal that indicates that the EffectChainSlot has been updated.
    void updated();


  private slots:
    void sendParameterUpdate();
    void slotChainEffectChanged(unsigned int effectSlotNumber);
    // Clears the effect in the given position in the loaded EffectChain.
    void slotClearEffect(unsigned int iEffectSlotNumber);

    void slotControlClear(double v);
    void slotControlChainSuperParameter(double v, bool force = false);
    void slotControlChainSelector(double v);
    void slotControlChainNextPreset(double v);
    void slotControlChainPrevPreset(double v);
    void slotChannelStatusChanged(const QString& group);

  private:
    QString debugString() const {
        return QString("EffectChainSlot(%1)").arg(m_group);
    }

    void addToEngine(EngineEffectRack* pRack, int iIndex);
    void removeFromEngine(EngineEffectRack* pRack, int iIndex);

    const unsigned int m_iChainSlotNumber;
    const QString m_group;
    EffectRack* m_pEffectRack;

    ControlPushButton* m_pControlClear;
    ControlObject* m_pControlNumEffects;
    ControlObject* m_pControlNumEffectSlots;
    ControlObject* m_pControlChainLoaded;
    ControlPushButton* m_pControlChainEnabled;
    ControlObject* m_pControlChainMix;
    ControlObject* m_pControlChainSuperParameter;
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
    QList<EffectSlotPointer> m_slots;
    QSignalMapper m_channelStatusMapper;
    EffectsManager* m_pEffectsManager;
    QString m_id;
    QString m_name;
    QString m_description;
    QSet<ChannelHandleAndGroup> m_enabledInputChannels;
    QList<EffectPointer> m_effects;
    EngineEffectChain* m_pEngineEffectChain;

    DISALLOW_COPY_AND_ASSIGN(EffectChainSlot);
};


#endif /* EFFECTCHAINSLOT_H */
