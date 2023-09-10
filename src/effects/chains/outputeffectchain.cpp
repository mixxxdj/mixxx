#include "effects/chains/outputeffectchain.h"

#include "control/control.h"
#include "effects/effectslot.h"
#include "moc_outputeffectchain.cpp"

OutputEffectChain::OutputEffectChain(EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : EffectChain(formatEffectChainGroup("[Main]"),
                  pEffectsManager,
                  pEffectsMessenger,
                  SignalProcessingStage::Postfader) {
    addEffectSlot("[OutputEffectRack_[Main]_Effect1]");
    ControlDoublePrivate::insertGroupAlias(formatEffectChainGroup("[Master]"), getGroup());
    ControlDoublePrivate::insertGroupAlias(
            QStringLiteral("[OutputEffectRack_[Master]_Effect1]"),
            QStringLiteral("[OutputEffectRack_[Main]_Effect1]"));

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
