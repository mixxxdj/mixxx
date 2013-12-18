#ifndef WEFFECTPARAMETER_H
#define WEFFECTPARAMETER_H

#include "widget/wlabel.h"
#include "effects/effectparameterslot.h"

class WEffectParameter : public WLabel {
    Q_OBJECT
  public:
    WEffectParameter(QWidget* pParent=NULL);
    virtual ~WEffectParameter();

    // Set the EffectParameterSlot that should be monitored by this
    // WEffectParameter.
    void setEffectParameterSlot(EffectParameterSlotPointer pEffectParameterSlot);

  private slots:
    void parameterUpdated();

  private:
    EffectParameterSlotPointer m_pEffectParameterSlot;
};


#endif /* WEFFECTPARAMETER_H */
