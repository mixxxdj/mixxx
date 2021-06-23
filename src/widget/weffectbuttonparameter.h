#pragma once

#include <QDomNode>

#include "widget/wlabel.h"
#include "widget/weffectparameterbase.h"
#include "skin/legacy/skincontext.h"

class EffectsManager;

class WEffectButtonParameter : public WEffectParameterBase {
    Q_OBJECT
  public:
    WEffectButtonParameter(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;
};
