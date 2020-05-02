#pragma once

#include <QDomNode>
#include <QComboBox>
#include "effects/effectslot.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffectSelector : public QComboBox, public WBaseWidget {
    Q_OBJECT
  public:
    WEffectSelector(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context);

  private slots:
    void slotEffectUpdated();
    void slotEffectSelected(int newIndex);
    void populate();

  private:
    int m_iEffectSlotIndex;
    EffectsManager* m_pEffectsManager;
    EffectSlotPointer m_pEffectSlot;
    EffectChainSlotPointer m_pChainSlot;
};
