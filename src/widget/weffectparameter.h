#ifndef WEFFECTPARAMETER_H
#define WEFFECTPARAMETER_H

#include <QDomNode>

#include "widget/wlabel.h"
#include "effects/effectparameterslot.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffectParameter : public WLabel {
    Q_OBJECT
  public:
    WEffectParameter(QWidget* pParent, EffectsManager* pEffectsManager);
    virtual ~WEffectParameter();

    void setup(QDomNode node, const SkinContext& context);

  private slots:
    void parameterUpdated();

  private:
    // Set the EffectParameterSlot that should be monitored by this
    // WEffectParameter.
    void setEffectParameterSlot(EffectParameterSlotPointer pEffectParameterSlot);

    EffectsManager* m_pEffectsManager;
    EffectParameterSlotPointer m_pEffectParameterSlot;
};


#endif /* WEFFECTPARAMETER_H */
