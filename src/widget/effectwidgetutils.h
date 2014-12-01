#ifndef EFFECTWIDGETUTILS_H
#define EFFECTWIDGETUTILS_H

#include <QDomNode>

#include "effects/effectsmanager.h"
#include "skin/skincontext.h"

class EffectWidgetUtils {
  public:
    static EffectRackPointer getEffectRackFromNode(
            const QDomNode& node,
            const SkinContext& context,
            EffectsManager* pEffectsManager) {
        if (pEffectsManager == NULL) {
            return EffectRackPointer();
        }

        // If specified, EffectRack always refers to a StandardEffectRack index.
        bool rackNumberOk = false;
        int rackNumber = context.selectInt(node, "EffectRack",
                                           &rackNumberOk);
        if (rackNumberOk) {
            // XML effect nodes are 1-indexed.
            return pEffectsManager->getStandardEffectRack(rackNumber - 1);
        }

        // For custom racks, users can specify EffectRackGroup explicitly
        // instead.
        QString rackGroup;
        if (!context.hasNodeSelectString(node, "EffectRackGroup", &rackGroup)) {
            return EffectRackPointer();
        }
        return pEffectsManager->getEffectRack(rackGroup);
    }

    static EffectChainSlotPointer getEffectChainSlotFromNode(
            const QDomNode& node,
            const SkinContext& context,
            EffectRackPointer pRack) {
        if (pRack.isNull()) {
            return EffectChainSlotPointer();
        }

        bool unitNumberOk = false;
        int unitNumber = context.selectInt(node, "EffectUnit",
                                           &unitNumberOk);
        if (unitNumberOk) {
            // XML effect nodes are 1-indexed.
            return pRack->getEffectChainSlot(unitNumber - 1);
        }

        QString unitGroup;
        if (!context.hasNodeSelectString(node, "EffectUnitGroup", &unitGroup)) {
            return EffectChainSlotPointer();
        }

        for (int i = 0; i < pRack->numEffectChainSlots(); ++i) {
            EffectChainSlotPointer pSlot = pRack->getEffectChainSlot(i);
            if (pSlot->getGroup() == unitGroup) {
                return pSlot;
            }
        }
        return EffectChainSlotPointer();
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
    EffectWidgetUtils() {};
};

#endif /* EFFECTWIDGETUTILS_H */
