#ifndef WEFFECT_H
#define WEFFECT_H

#include "widget/wlabel.h"
#include "effects/effectslot.h"

class WEffect : public WLabel {
    Q_OBJECT
  public:
    WEffect(QWidget* pParent=NULL);
    virtual ~WEffect();

    // Set the EffectSlot that should be monitored by this WEffect.
    void setEffectSlot(EffectSlotPointer pEffectSlot);

  private slots:
    void effectUpdated();

  private:
    EffectSlotPointer m_pEffectSlot;
};


#endif /* WEFFECT_H */
