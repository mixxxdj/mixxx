#pragma once

#include <QDomNode>

#include "skin/legacy/skincontext.h"
#include "widget/weffectparameternamebase.h"
#include "widget/wlabel.h"

class EffectsManager;

class WEffectKnobParameterName : public WEffectParameterNameBase {
    Q_OBJECT
  public:
    WEffectKnobParameterName(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;
};
