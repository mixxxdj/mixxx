#include "effects/chains/outputeffectchain.h"

#include "effects/effectslot.h"

OutputEffectChain::OutputEffectChain(EffectsManager* pEffectsManager,
        EffectsMessengerPointer pEffectsMessenger)
        : EffectChain(formatEffectChainGroup("[Master]"),
                  pEffectsManager,
                  pEffectsMessenger,
                  SignalProcessingStage::Postfader) {
    addEffectSlot("[OutputEffectRack_[Master]_Effect1]");
    m_effectSlots[0]->setEnabled(true);

    // Register the master channel
    GroupHandle masterHandleAndGroup = nullptr;

    // TODO(Be): Remove this hideous hack to get the GroupHandle
    const QSet<GroupHandle>& registeredChannels =
            m_pEffectsManager->registeredInputChannels();
    for (GroupHandle handle_group : registeredChannels) {
        if (nameOfGroupHandle(handle_group) == "[MasterOutput]") {
            masterHandleAndGroup = handle_group;
            break;
        }
    }
    DEBUG_ASSERT(masterHandleAndGroup != nullptr);

    registerInputChannel(masterHandleAndGroup);
    enableForInputChannel(masterHandleAndGroup);
    m_pControlChainMix->set(1.0);
    sendParameterUpdate();
}

QString OutputEffectChain::formatEffectChainGroup(
        const QString& group) {
    return QString("[OutputEffectRack_%1]").arg(group);
}
