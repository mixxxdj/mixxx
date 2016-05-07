#ifndef WEFFECTBUTTONPARAMETER_H
#define WEFFECTBUTTONPARAMETER_H

#include <QDomNode>

#include "widget/wlabel.h"
#include "widget/weffectparameterbase.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffectButtonParameter : public WEffectParameterBase {
    Q_OBJECT
  public:
    WEffectButtonParameter(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;
};


#endif /* WEFFECTBUTTONPARAMETER_H */
