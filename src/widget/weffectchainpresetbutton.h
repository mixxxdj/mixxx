#pragma once

#include <QMenu>
#include <QPushButton>

#include "effects/effectsmanager.h"
#include "skin/skincontext.h"
#include "util/parented_ptr.h"
#include "widget/wbasewidget.h"

class WEffectChainPresetButton : public QPushButton, public WBaseWidget {
    Q_OBJECT
  public:
    WEffectChainPresetButton(QWidget* parent, EffectsManager* pEffectsManager);

    void setup(const QDomNode& node, const SkinContext& context);

  private slots:
    void populateMenu();
    void saveChainPreset();

  private:
    int m_iChainNumber;
    EffectsManager* m_pEffectsManager;
    parented_ptr<QMenu> m_pMenu;
};
