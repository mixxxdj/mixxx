#pragma once

#include "effects/defs.h"
#include "widget/wlabel.h"

class EffectsManager;

class WEffectName : public WLabel {
    Q_OBJECT
  public:
    WEffectName(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context) override;

  private slots:
    void effectUpdated();

  private:
    void setEffectSlot(EffectSlotPointer pEffectSlot);

    EffectsManager* m_pEffectsManager;
    EffectSlotPointer m_pEffectSlot;
};
