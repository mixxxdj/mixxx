#pragma once

#include <QAction>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QPushButton>
#include <QScopedPointer>

#include "control/controlproxy.h"
#include "preferences/configobject.h"
#include "preferences/usersettings.h"
#include "skin/skincontext.h"
#include "util/parented_ptr.h"
#include "widget/wbasewidget.h"

class WMainMenu;

class WMainMenuBarButton : public QPushButton, public WBaseWidget {
    Q_OBJECT
  public:
    WMainMenuBarButton(QWidget* pParent,
            WMainMenu* pMainMenu,
            ConfigObject<ConfigValueKbd>* pKbdConfig);

  private:
    void initialize(WMainMenu* pMainMenu);

    const UserSettingsPointer m_pConfig;
    const parented_ptr<QMenu> m_pMenu;
};
