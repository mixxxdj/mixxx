#pragma once

#include <QMenu>
#include <QPushButton>

#include "effects/presets/effectchainpresetmanager.h"
#include "util/parented_ptr.h"
#include "widget/wbasewidget.h"

class QDomNode;
class SkinContext;
class EffectsManager;

class WEffectChainPresetButton : public QPushButton, public WBaseWidget {
    Q_OBJECT
  public:
    WEffectChainPresetButton(QWidget* parent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context);

  private slots:
    void populateMenu();

  private:
    bool event(QEvent* pEvent);
    EffectChainPointer m_pChain;

    EffectsManager* m_pEffectsManager;
    EffectChainPresetManagerPointer m_pChainPresetManager;
    parented_ptr<QMenu> m_pMenu;
};
