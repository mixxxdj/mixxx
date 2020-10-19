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
#include "widget/wbasewidget.h"

class WMainMenuBar;

class WMainMenuBarButton : public QPushButton, public WBaseWidget {
    Q_OBJECT
  public:
    WMainMenuBarButton(QWidget* pParent, WMainMenuBar* pMainMenu);
    void setup(const QDomNode& node, const SkinContext& context);

  private:
    void initialize(WMainMenuBar* pMainMenu);

    UserSettingsPointer m_pConfig;
    QMenu* m_pMenu;
};
