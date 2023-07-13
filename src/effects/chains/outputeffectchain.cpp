#include "effects/chains/outputeffectchain.h"

#include "effects/effectslot.h"
#include "moc_outputeffectchain.cpp"

OutputEffectChain::OutputEffectChain(EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : EffectChain(formatEffectChainGroup("[Master]"),
                  pEffectsManager,
                  std::move(pEffectsMessenger),
                  SignalProcessingStage::Postfader) {
    addEffectSlot("[OutputEffectRack_[Master]_Effect1]");
    m_effectSlots[0]->setEnabled(true);

    // Register the master channel
    const ChannelHandleAndGroup* masterHandleAndGroup = nullptr;

    // TODO(Be): Remove this hideous hack to get the ChannelHandleAndGroup
    const QSet<ChannelHandleAndGroup>& registeredChannels =
            m_pEffectsManager->registeredInputChannels();
    for (const ChannelHandleAndGroup& handle_group : registeredChannels) {
        if (handle_group.name() == "[MasterOutput]") {
            masterHandleAndGroup = &handle_group;
            break;
        }
    }
    DEBUG_ASSERT(masterHandleAndGroup != nullptr);

    registerInputChannel(*masterHandleAndGroup);
    enableForInputChannel(*masterHandleAndGroup);
    m_pControlChainMix->set(1.0);
    sendParameterUpdate();
}

QString OutputEffectChain::formatEffectChainGroup(
        const QString& group) {
    return QString("[OutputEffectRack_%1]").arg(group);
}
