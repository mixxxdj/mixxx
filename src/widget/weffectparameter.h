#ifndef WEFFECTPARAMETER_H
#define WEFFECTPARAMETER_H

#include <QDomNode>

#include "widget/wlabel.h"
#include "widget/weffectparameterbase.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffectParameter : public WEffectParameterBase {
    Q_OBJECT
  public:
    WEffectParameter(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;
};


#endif /* WEFFECTPARAMETER_H */
