#pragma once

#include <QDomNode>

#include "widget/wlabel.h"
#include "effects/effectslot.h"
#include "skin/skincontext.h"

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
