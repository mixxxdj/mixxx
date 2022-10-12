#pragma once

#include "control/pollingcontrolproxy.h"
#include "effects/chains/pergroupeffectchain.h"

/// EqualizerEffectChain is specifically for the equalizers only.
/// It only has a single effect in the chain. The user can pick the
/// effect in the preferences. In the main GUI, only the parameters of
/// that single effect are presented, not the metaknob nor superknob.
/// EqualizerEffectChain is hardwired to one input channel, always
/// enabled, and has the mix knob fully enabled.
class EqualizerEffectChain : public PerGroupEffectChain {
    Q_OBJECT
  public:
    EqualizerEffectChain(const QString& group,
            EffectsManager* pEffectsManager,
            EffectsMessengerPointer pEffectsMessenger);

    void setFilterWaveform(bool state);

    static QString formatEffectChainGroup(const QString& group);
    static QString formatEffectSlotGroup(const QString& group);

  private slots:
    void slotFilterLowKillChanged(double value);
    void slotFilterMidKillChanged(double value);
    void slotFilterHighKillChanged(double value);

  private:
    void setupAliasesForGroup(const QString& group);
    PollingControlProxy m_eqButtonMode;

    std::unique_ptr<ControlObject> m_pCOFilterWaveform;
    std::unique_ptr<ControlPushButton> m_pCOFilterLowKill;
    std::unique_ptr<ControlPushButton> m_pCOFilterMidKill;
    std::unique_ptr<ControlPushButton> m_pCOFilterHighKill;
};
