#include "widget/effectwidgetutils.h"
#include "widget/weffectparameterknobcomposed.h"

namespace {
const QString effectGroupSeparator = "_";
const QString groupClose = "]";
} // anonymous namespace

void WEffectParameterKnobComposed::setupEffectParameterSlot(const ConfigKey& configKey) {
    QStringList parts = configKey.group.split(effectGroupSeparator);
    QRegExp intRegEx(".*(\\d+).*");

    EffectRackPointer pRack = m_pEffectsManager->getEffectRack(parts.at(0) + groupClose);
    VERIFY_OR_DEBUG_ASSERT(pRack) {
        return;
    }

    EffectChainSlotPointer pChainSlot;
    if (parts.at(0) == "[EffectRack1") {
        intRegEx.indexIn(parts.at(1));
        pChainSlot = pRack->getEffectChainSlot(intRegEx.cap(1).toInt() - 1);
    } else {
        // Assume a PerGroupRack
        const QString chainGroup =
                parts.at(0) + effectGroupSeparator + parts.at(1) + groupClose;
        for (int i = 0; i < pRack->numEffectChainSlots(); ++i) {
            EffectChainSlotPointer pSlot = pRack->getEffectChainSlot(i);
            if (pSlot->getGroup() == chainGroup) {
                pChainSlot = pSlot;
                break;
            }
        }
    }
    VERIFY_OR_DEBUG_ASSERT(pChainSlot) {
        return;
    }

    intRegEx.indexIn(parts.at(2));
    EffectSlotPointer pEffectSlot =
            pChainSlot->getEffectSlot(intRegEx.cap(1).toInt() - 1);
    VERIFY_OR_DEBUG_ASSERT(pEffectSlot) {
        return;
    }

    intRegEx.indexIn(configKey.item);
    EffectParameterSlotBasePointer pParameterSlot =
            pEffectSlot->getEffectParameterSlot(intRegEx.cap(1).toInt() - 1);
    VERIFY_OR_DEBUG_ASSERT(pParameterSlot) {
        return;
    }
    setEffectParameterSlot(pParameterSlot);
}

void WEffectParameterKnobComposed::setEffectParameterSlot(
        EffectParameterSlotBasePointer pParameterSlot) {
    m_pEffectParameterSlot = pParameterSlot;
    if (m_pEffectParameterSlot) {
        connect(m_pEffectParameterSlot.data(), SIGNAL(updated()),
                this, SLOT(parameterUpdated()));
    }
    parameterUpdated();
}

void WEffectParameterKnobComposed::parameterUpdated() {
    if (m_pEffectParameterSlot) {
        setBaseTooltip(QString("%1\n%2").arg(
                       m_pEffectParameterSlot->name(),
                       m_pEffectParameterSlot->description()));
    } else {
        // The knob should be hidden by the skin when the parameterX_loaded ControlObject
        // indicates no parameter is loaded, so this tooltip should never be shown.
        setBaseTooltip("");
    }
}
