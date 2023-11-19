#pragma once

#include "widget/weffectparameternamebase.h"

class EffectsManager;

class WEffectButtonParameterName : public WEffectParameterNameBase {
    Q_OBJECT
  public:
    WEffectButtonParameterName(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;
};
