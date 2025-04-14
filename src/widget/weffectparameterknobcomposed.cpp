#include "widget/weffectparameterknobcomposed.h"

#include "effects/effectparameterslotbase.h"
#include "effects/presets/effectchainpreset.h"
#include "moc_weffectparameterknobcomposed.cpp"
#include "widget/effectwidgetutils.h"
//// Included this headers for QToolTip, label access, and manifest for dynamic tooltip display.
#include <QToolTip>

#include "effects/backends/effectmanifestparameter.h"
#include "widget/weffectknobparametername.h"

void WEffectParameterKnobComposed::setup(const QDomNode& node, const SkinContext& context) {
    WKnobComposed::setup(node, context);
    auto pChainSlot = EffectWidgetUtils::getEffectChainFromNode(
            node, context, m_pEffectsManager);
    auto pEffectSlot =
            EffectWidgetUtils::getEffectSlotFromNode(node, context, pChainSlot);
    m_pEffectParameterSlot = EffectWidgetUtils::getParameterSlotFromNode(
            node, context, pEffectSlot);
    VERIFY_OR_DEBUG_ASSERT(m_pEffectParameterSlot) {
        SKIN_WARNING(node, context, QStringLiteral("Could not find effect parameter slot"));
        return;
    }
    connect(m_pEffectParameterSlot.data(),
            &EffectParameterSlotBase::updated,
            this,
            &WEffectParameterKnobComposed::parameterUpdated);
    parameterUpdated();
}

void WEffectParameterKnobComposed::parameterUpdated() {
    if (m_pEffectParameterSlot->isLoaded()) {
        setBaseTooltip(QStringLiteral("%1\n%2").arg(
                m_pEffectParameterSlot->name(),
                m_pEffectParameterSlot->description()));
        setDefaultAngleFromParameterOrReset(m_pEffectParameterSlot->neutralPointOnScale());
    } else {
        // The knob should be hidden by the skin when the parameterX_loaded ControlObject
        // indicates no parameter is loaded, so this tooltip should never be shown.
        setBaseTooltip("");
    }
}

// Override mousePressEvent to show a tooltip with the parameter name and its
// current value (with correct unit). Falls back to label text if unavailable.
void WEffectParameterKnobComposed::mousePressEvent(QMouseEvent* e) {
    WKnobComposed::mousePressEvent(e);

    if (m_pEffectParameterSlot && m_pEffectParameterSlot->isLoaded()) {
        EffectManifestParameterPointer pManifest = m_pEffectParameterSlot->getManifest();
        if (pManifest) {
            QString name = pManifest->name();
            double value = m_pEffectParameterSlot->currentParameterValue();
            QString valueStr = pManifest->valueToString(value);

            QString tooltip = QString("%1: %2").arg(name, valueStr);
            QToolTip::showText(e->globalPosition().toPoint(), tooltip, this);
            return;
        }
    }
    // fallback: show label
    if (m_pLabel) {
        QString labelText = m_pLabel->text();
        if (!labelText.isEmpty()) {
            QToolTip::showText(e->globalPosition().toPoint(), labelText, this);
        }
    }
}
