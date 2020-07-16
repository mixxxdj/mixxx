#pragma once

#include <QDomNode>

#include "widget/wlabel.h"
#include "widget/weffectparameternamebase.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffectKnobParameterName : public WEffectParameterNameBase {
    Q_OBJECT
  public:
    WEffectKnobParameterName(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;
};
