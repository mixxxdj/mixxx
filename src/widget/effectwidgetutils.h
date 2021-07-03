#pragma once

#include <QDomNode>

#include "effects/defs.h"
#include "effects/effectchain.h"
#include "effects/effectslot.h"
#include "effects/effectsmanager.h"
#include "skin/legacy/skincontext.h"

class EffectWidgetUtils {
  public:
    static int getEffectUnitNumberFromNode(const QDomNode& node, const SkinContext& context) {
        bool unitNumberOk = false;
        int unitNumber = context.selectInt(node, "EffectUnit", &unitNumberOk);
        if (unitNumberOk) {
            // XML effect nodes are 1-indexed.
            return unitNumber - 1;
        }
        return 0;
    }

    static EffectChainPointer getEffectChainFromNode(
            const QDomNode& node,
            const SkinContext& context,
            EffectsManager* pEffectsManager) {
        if (pEffectsManager == nullptr) {
            return EffectChainPointer();
        }

        bool unitNumberOk = false;
        int unitNumber = context.selectInt(node, "EffectUnit",
                                           &unitNumberOk);
        if (unitNumberOk) {
            // XML effect nodes are 1-indexed.
            return pEffectsManager->getStandardEffectChain(unitNumber - 1);
        }

        QString unitGroup;
        if (!context.hasNodeSelectString(node, "EffectUnitGroup", &unitGroup)) {
            return EffectChainPointer();
        }

        return pEffectsManager->getEffectChain(unitGroup);
    }

    static int getEffectSlotIndexFromNode(
            const QDomNode& node,
            const SkinContext& context) {
        bool effectSlotOk = false;
        int effectSlotIndex = context.selectInt(node, "Effect", &effectSlotOk);
        if (effectSlotOk) {
            // XML effect nodes are 1-indexed.
            return effectSlotIndex - 1;
        }
        return -1;
    }

    static EffectSlotPointer getEffectSlotFromNode(
            const QDomNode& node,
            const SkinContext& context,
            EffectChainPointer pChainSlot) {
        if (pChainSlot.isNull()) {
            return EffectSlotPointer();
        }

        int effectSlotIndex = getEffectSlotIndexFromNode(node, context);
        return pChainSlot->getEffectSlot(effectSlotIndex);
    }

    static EffectParameterSlotBasePointer getParameterSlotFromNode(
            const QDomNode& node,
            const SkinContext& context,
            EffectSlotPointer pEffectSlot) {
        if (pEffectSlot.isNull()) {
            return EffectParameterSlotBasePointer();
        }
        bool parameterNumberOk = false;
        int parameterNumber = context.selectInt(node, "EffectParameter",
                                                &parameterNumberOk);
        if (parameterNumberOk) {
            // XML effect nodes are 1-indexed.
            return pEffectSlot->getEffectParameterSlot(
                    EffectManifestParameter::ParameterType::Knob,
                    parameterNumber - 1);
        }
        return EffectParameterSlotBasePointer();
    }

    static EffectParameterSlotBasePointer getButtonParameterSlotFromNode(
            const QDomNode& node,
            const SkinContext& context,
            EffectSlotPointer pEffectSlot) {
        if (pEffectSlot.isNull()) {
            return EffectParameterSlotBasePointer();
        }
        bool parameterNumberOk = false;
        int parameterNumber = context.selectInt(node, "EffectButtonParameter",
                                                &parameterNumberOk);
        if (parameterNumberOk) {
            // XML effect nodes are 1-indexed.
            return pEffectSlot->getEffectParameterSlot(
                    EffectManifestParameter::ParameterType::Button,
                    parameterNumber - 1);
        }
        return EffectParameterSlotBasePointer();
    }

  private:
    EffectWidgetUtils() = default;
};
