#pragma once

#include <QDomNode>

#include "widget/wlabel.h"
#include "effects/effectparameterslotbase.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffectParameterNameBase : public WLabel {
    Q_OBJECT
  public:
    WEffectParameterNameBase(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override = 0;

  protected slots:
    void parameterUpdated();

  protected:
    void setEffectParameterSlot(EffectParameterSlotBasePointer pEffectKnobParameterSlot);

    EffectsManager* m_pEffectsManager;
    EffectParameterSlotBasePointer m_pEffectParameterSlot;
};
