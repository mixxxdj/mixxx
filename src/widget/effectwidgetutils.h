#ifndef EFFECTWIDGETUTILS_H
#define EFFECTWIDGETUTILS_H

#include <QDomNode>

#include "effects/effectsmanager.h"
#include "effects/defs.h"
#include "effects/effectslot.h"
#include "skin/skincontext.h"

class EffectWidgetUtils {
  public:
    static EffectChainSlotPointer getEffectChainSlotFromNode(
            const QDomNode& node,
            const SkinContext& context,
            EffectsManager* pEffectsManager) {
        if (pEffectsManager == nullptr) {
            return EffectChainSlotPointer();
        }

        bool unitNumberOk = false;
        int unitNumber = context.selectInt(node, "EffectUnit",
                                           &unitNumberOk);
        if (unitNumberOk) {
            // XML effect nodes are 1-indexed.
            return pEffectsManager->getStandardEffectChainSlot(unitNumber - 1);
        }

        QString unitGroup;
        if (!context.hasNodeSelectString(node, "EffectUnitGroup", &unitGroup)) {
            return EffectChainSlotPointer();
        }

        return pEffectsManager->getEffectChainSlot(unitGroup);
    }

    static EffectSlotPointer getEffectSlotFromNode(
            const QDomNode& node,
            const SkinContext& context,
            EffectChainSlotPointer pChainSlot) {
        if (pChainSlot.isNull()) {
            return EffectSlotPointer();
        }

        bool effectSlotOk = false;
        int effectSlot = context.selectInt(node, "Effect", &effectSlotOk);
        if (effectSlotOk) {
            // XML effect nodes are 1-indexed.
            return pChainSlot->getEffectSlot(effectSlot - 1);
        }
        return EffectSlotPointer();
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
            return pEffectSlot->getEffectParameterSlot(parameterNumber - 1);
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
            return pEffectSlot->getEffectButtonParameterSlot(parameterNumber - 1);
        }
        return EffectParameterSlotBasePointer();
    }

  private:
    EffectWidgetUtils() = default;
};

#endif /* EFFECTWIDGETUTILS_H */
