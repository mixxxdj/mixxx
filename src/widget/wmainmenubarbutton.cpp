#include "widget/wmainmenubarbutton.h"

#include <QAction>
#include <QMenu>

#include "widget/wmainmenu.h"

WMainMenuBarButton::WMainMenuBarButton(QWidget* pParent,
        WMainMenu* pMainMenu,
        ConfigObject<ConfigValueKbd>* pKbdConfig)
        : QPushButton("...", pParent),
          WBaseWidget(this),
          m_pMenu(make_parented<QMenu>(this)) {
    initialize(pMainMenu);
    auto shortcut = QKeySequence(pKbdConfig->getValue(
            ConfigKey("[KeyboardShortcuts]", "Hamburger_Open"),
            tr("F10")));
    setShortcut(shortcut);
    setToolTip(tr("Mainmenu") + "\n" +
            tr("Shortcut") + ": " + shortcut.toString(QKeySequence::NativeText));
}

void WMainMenuBarButton::initialize(WMainMenu* pMainMenu) {
    setMenu(m_pMenu);
    pMainMenu->createMenu([this](QMenu* menu, QAction* action, bool separator) {
        if (menu) {
            m_pMenu->addMenu(menu);
        } else if (action) {
            m_pMenu->addAction(action);
        } else if (separator) {
            m_pMenu->addSeparator();
        }
    },
            this);
}
