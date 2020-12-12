#pragma once

#include <QDomNode>

#include "widget/wlabel.h"
#include "effects/effectparameterslotbase.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffectParameterBase : public WLabel {
    Q_OBJECT
  public:
    WEffectParameterBase(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override = 0;

  protected slots:
    void parameterUpdated();

  protected:
    // Set the EffectParameterSlot that should be monitored by this
    // WEffectParameterBase.
    void setEffectParameterSlot(EffectParameterSlotBasePointer pEffectParameterSlot);

    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
};
