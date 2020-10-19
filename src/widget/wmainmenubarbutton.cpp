#include "widget/wmainmenubarbutton.h"
#include "widget/wmainmenubar.h"

WMainMenuBarButton::WMainMenuBarButton(QWidget* pParent, WMainMenuBar* pMainMenu)
        : QPushButton("...", pParent),
          WBaseWidget(this),
          m_pMenu(nullptr) {
    initialize(pMainMenu);
}

void WMainMenuBarButton::setup(const QDomNode& node, const SkinContext& context) {
}

void WMainMenuBarButton::initialize(WMainMenuBar* pMainMenu) {
    m_pMenu = new QMenu(this);
    setMenu(m_pMenu);
    pMainMenu->createMenu([=](QMenu* x) { this->m_pMenu->addMenu(x); });
}
