#ifndef WEFFECTBUTTONPARAMETER_H
#define WEFFECTBUTTONPARAMETER_H

#include <QByteArrayData>
#include <QDomNode>
#include <QString>

#include "skin/skincontext.h"
#include "widget/weffectparameterbase.h"
#include "widget/wlabel.h"

class EffectsManager;
class QObject;
class QWidget;

class WEffectButtonParameter : public WEffectParameterBase {
    Q_OBJECT
  public:
    WEffectButtonParameter(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;
};


#endif /* WEFFECTBUTTONPARAMETER_H */
