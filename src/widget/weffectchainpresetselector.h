#pragma once

#include <QComboBox>
#include <QDomNode>

#include "effects/effectchain.h"
#include "skin/legacy/skincontext.h"

class EffectsManager;

class WEffectChainPresetSelector : public QComboBox, public WBaseWidget {
    Q_OBJECT
  public:
    WEffectChainPresetSelector(QWidget* pParent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context);

  private slots:
    void populate();
    void slotEffectChainPresetSelected(int index);
    void slotChainPresetChanged(const QString& name);
    bool event(QEvent* pEvent) override;
    void paintEvent(QPaintEvent* e) override;

  private:
    bool m_bQuickEffectChain;
    EffectChainPresetManagerPointer m_pChainPresetManager;
    EffectsManager* m_pEffectsManager;
    EffectChainPointer m_pChain;
};
