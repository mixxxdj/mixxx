#include "effects/chains/standardeffectchain.h"

#include "effects/effectsmanager.h"
#include "mixer/playermanager.h"
#include "moc_standardeffectchain.cpp"

StandardEffectChain::StandardEffectChain(unsigned int iChainNumber,
        EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : EffectChain(formatEffectChainGroup(iChainNumber),
                  pEffectsManager,
                  pEffectsMessenger,
                  SignalProcessingStage::Postfader) {
    for (int i = 0; i < kNumEffectsPerUnit; ++i) {
        addEffectSlot(formatEffectSlotGroup(iChainNumber, i));
    }

    const QSet<ChannelHandleAndGroup>& registeredChannels =
            m_pEffectsManager->registeredInputChannels();
    for (const ChannelHandleAndGroup& handle_group : registeredChannels) {
        int deckNumber;
        if (PlayerManager::isDeckGroup(handle_group.name(), &deckNumber) &&
                (iChainNumber + 1) == (unsigned)deckNumber) {
            registerInputChannel(handle_group, 1.0);
        } else {
            registerInputChannel(handle_group, 0.0);
        }
    }
}

QString StandardEffectChain::formatEffectChainGroup(const int iChainNumber) {
    // EffectRacks never did anything and there was never more than one of them,
    // but it remains in the ControlObject group name for backwards compatibility.
    return QString("[EffectRack1_EffectUnit%1]")
            .arg(QString::number(iChainNumber + 1));
}

QString StandardEffectChain::formatEffectSlotGroup(const int iChainSlotNumber,
        const int iEffectSlotNumber) {
    return QString("[EffectRack1_EffectUnit%1_Effect%2]")
            .arg(QString::number(iChainSlotNumber + 1),
                    QString::number(iEffectSlotNumber + 1));
}
