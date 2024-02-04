#include "effects/chains/outputeffectchain.h"

#include "effects/effectslot.h"
#include "effects/effectsmanager.h"
#include "moc_outputeffectchain.cpp"

OutputEffectChain::OutputEffectChain(EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : EffectChain(formatEffectChainGroup("[Master]"),
                  pEffectsManager,
                  pEffectsMessenger,
                  SignalProcessingStage::Postfader) {
    addEffectSlot("[OutputEffectRack_[Master]_Effect1]");
    m_effectSlots[0]->setEnabled(true);

    // Register the main channel
    const ChannelHandleAndGroup* mainHandleAndGroup = nullptr;

    // TODO(Be): Remove this hideous hack to get the ChannelHandleAndGroup
    const QSet<ChannelHandleAndGroup>& registeredChannels =
            m_pEffectsManager->registeredInputChannels();
    for (const ChannelHandleAndGroup& handle_group : registeredChannels) {
        if (handle_group.name() == "[MasterOutput]") {
            mainHandleAndGroup = &handle_group;
            break;
        }
    }
    DEBUG_ASSERT(mainHandleAndGroup != nullptr);

    registerInputChannel(*mainHandleAndGroup);
    enableForInputChannel(*mainHandleAndGroup);
    m_pControlChainMix->set(1.0);
    sendParameterUpdate();
}

QString OutputEffectChain::formatEffectChainGroup(
        const QString& group) {
    return QString("[OutputEffectRack_%1]").arg(group);
}
