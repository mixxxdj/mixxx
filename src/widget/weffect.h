#ifndef WEFFECT_H
#define WEFFECT_H

#include <QDomNode>

#include "widget/wlabel.h"
#include "effects/effectslot.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffect : public WLabel {
    Q_OBJECT
  public:
    WEffect(QWidget* pParent, EffectsManager* pEffectsManager);
    virtual ~WEffect();

    void setup(QDomNode node, const SkinContext& context);

  private slots:
    void effectUpdated();

  private:
    // Set the EffectSlot that should be monitored by this WEffect.
    void setEffectSlot(EffectSlotPointer pEffectSlot);

    EffectsManager* m_pEffectsManager;
    EffectSlotPointer m_pEffectSlot;
};


#endif /* WEFFECT_H */
