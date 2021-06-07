#pragma once

#include <QObject>
#include <QMap>
#include <QList>

#include "engine/channelhandle.h"
#include "util/class.h"
#include "effects/effectchain.h"

class ControlObject;
class ControlPushButton;
class ControlEncoder;
class EffectChainSlot;

class EffectChainSlot : public QObject {
    Q_OBJECT
  public:
    EffectChainSlot(EffectRack* pRack,
                    const QString& group,
                    const unsigned int iChainNumber);
    virtual ~EffectChainSlot();

    // Get the ID of the loaded EffectChain
    QString id() const;

    unsigned int numSlots() const;
    EffectSlotPointer addEffectSlot(const QString& group);
    EffectSlotPointer getEffectSlot(unsigned int slotNumber);

    void loadEffectChainToSlot(EffectChainPointer pEffectChain);
    void updateRoutingSwitches();
    EffectChainPointer getEffectChain() const;
    EffectChainPointer getOrCreateEffectChain(EffectsManager* pEffectsManager);

    void registerInputChannel(const ChannelHandleAndGroup& handleGroup);

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

  signals:
    // Indicates that the effect pEffect has been loaded into slotNumber of
    // EffectChainSlot chainNumber. pEffect may be an invalid pointer, which
    // indicates that a previously loaded effect was removed from the slot.
    void effectLoaded(EffectPointer pEffect, unsigned int chainNumber,
                      unsigned int slotNumber);

    // Indicates that the given EffectChain was loaded into this
    // EffectChainSlot
    void effectChainLoaded(EffectChainPointer pEffectChain);

    // Signal that whoever is in charge of this EffectChainSlot should load the
    // next EffectChain into it.
    void nextChain(unsigned int iChainSlotNumber,
                   EffectChainPointer pEffectChain);

    // Signal that whoever is in charge of this EffectChainSlot should load the
    // previous EffectChain into it.
    void prevChain(unsigned int iChainSlotNumber,
                   EffectChainPointer pEffectChain);

    // Signal that whoever is in charge of this EffectChainSlot should clear
    // this EffectChain (by removing the chain from this EffectChainSlot).
    void clearChain(unsigned int iChainNumber, EffectChainPointer pEffectChain);

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
    void slotChainEffectChanged(unsigned int effectSlotNumber, bool shouldEmit);
    void slotChainNameChanged(const QString& name);
    void slotChainEnabledChanged(bool enabled);
    void slotChainMixChanged(double mix);
    void slotChainMixModeChanged(EffectChainMixMode mixMode);
    void slotChainChannelStatusChanged(const QString& group, bool enabled);

    void slotEffectLoaded(EffectPointer pEffect, unsigned int slotNumber);
    // Clears the effect in the given position in the loaded EffectChain.
    void slotClearEffect(unsigned int iEffectSlotNumber);

    void slotControlClear(double v);
    void slotControlChainEnabled(double v);
    void slotControlChainMix(double v);
    void slotControlChainSuperParameter(double v, bool force);
    void slotControlChainMixMode(double v);
    void slotControlChainSelector(double v);
    void slotControlChainNextPreset(double v);
    void slotControlChainPrevPreset(double v);
    void slotChannelStatusChanged(const QString& group);

  private:
    QString debugString() const {
        return QString("EffectChainSlot(%1)").arg(m_group);
    }

    const unsigned int m_iChainSlotNumber;
    const QString m_group;
    EffectRack* m_pEffectRack;

    EffectChainPointer m_pEffectChain;

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
    ControlPushButton* m_pControlChainHasControllerFocus;
    ControlPushButton* m_pControlChainShowParameters;
    ControlPushButton* m_pControlChainFocusedEffect;

    struct ChannelInfo {
        // Takes ownership of pEnabled.
        ChannelInfo(const ChannelHandleAndGroup& handleGroup, ControlObject* pEnabled)
                : handleGroup(handleGroup),
                  pEnabled(pEnabled) {
        }
        ~ChannelInfo() {
            delete pEnabled;
        }
        ChannelHandleAndGroup handleGroup;
        ControlObject* pEnabled;
    };
    QMap<QString, ChannelInfo*> m_channelInfoByName;

    QList<EffectSlotPointer> m_slots;

    DISALLOW_COPY_AND_ASSIGN(EffectChainSlot);
};
