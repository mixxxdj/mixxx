#include <QAction>
#include <QMenu>

#include "widget/wmainmenubarbutton.h"
#include "widget/wmainmenubar.h"

WMainMenuBarButton::WMainMenuBarButton(QWidget* pParent,
        WMainMenuBar* pMainMenu,
        ConfigObject<ConfigValueKbd>* pKbdConfig)
        : QPushButton("...", pParent),
          WBaseWidget(this),
          m_pMenu(make_parented<QMenu>(this)) {
    initialize(pMainMenu);
    setShortcut(
            QKeySequence(pKbdConfig->getValue(
                    ConfigKey("[KeyboardShortcuts]", "Hamburger_Open"),
                    tr("F10"))));
}

void WMainMenuBarButton::initialize(WMainMenuBar* pMainMenu) {
    setMenu(m_pMenu);
    pMainMenu->createMenu([this](QMenu* menu, QAction* action, bool separator) {
        if (menu) {
            m_pMenu->addMenu(menu);
        } else if (action) {
            m_pMenu->addAction(action);
        } else if (separator) {
            m_pMenu->addSeparator();
        }
    });
}
