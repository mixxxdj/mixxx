#ifndef WEFFECTPARAMETER_H
#define WEFFECTPARAMETER_H

#include <QByteArrayData>
#include <QDomNode>
#include <QString>

#include "skin/skincontext.h"
#include "widget/weffectparameterbase.h"
#include "widget/wlabel.h"

class EffectsManager;
class QObject;
class QWidget;

class WEffectParameter : public WEffectParameterBase {
    Q_OBJECT
  public:
    WEffectParameter(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;
};


#endif /* WEFFECTPARAMETER_H */
