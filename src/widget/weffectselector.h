#pragma once

#include <QComboBox>
#include <QDomNode>

#include "effects/effectslot.h"
#include "skin/legacy/skincontext.h"

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
    bool event(QEvent* pEvent) override;

  private:
    EffectsManager* m_pEffectsManager;
    VisibleEffectsListPointer m_pVisibleEffectsList;
    EffectSlotPointer m_pEffectSlot;
};
