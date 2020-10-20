#include "widget/wmainmenubarbutton.h"
#include "widget/wmainmenubar.h"

WMainMenuBarButton::WMainMenuBarButton(QWidget* pParent, WMainMenuBar* pMainMenu)
        : QPushButton("...", pParent),
          WBaseWidget(this),
          m_pMenu(make_parented<QMenu>(this)) {
    initialize(pMainMenu);
}

void WMainMenuBarButton::initialize(WMainMenuBar* pMainMenu) {
    setMenu(m_pMenu);
    pMainMenu->createMenu([this](QMenu* x) { m_pMenu->addMenu(x); });
}
