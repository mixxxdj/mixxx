#pragma once

#include <QComboBox>
#include <QDomNode>

#include "effects/effectchainslot.h"
#include "skin/skincontext.h"

class EffectsManager;

class WEffectChainPresetSelector : public QComboBox, public WBaseWidget {
    Q_OBJECT
  public:
    WEffectChainPresetSelector(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context);

  private slots:
    void populate();
    void slotEffectChainPresetSelected(int index);
    void slotEffectChainNameChanged(const QString& name);

  private:
    bool m_bQuickEffectChain;
    EffectChainPresetManagerPointer m_pChainPresetManager;
    EffectsManager* m_pEffectsManager;
    EffectChainSlotPointer m_pChainSlot;
};
