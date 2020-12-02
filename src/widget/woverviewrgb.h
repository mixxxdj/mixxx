#ifndef WOVERVIEWRGB_H
#define WOVERVIEWRGB_H

#include <QString>

#include "preferences/usersettings.h"
#include "widget/woverview.h"

class PlayerManager;
class QWidget;

class WOverviewRGB : public WOverview {
  public:
    WOverviewRGB(
            const QString& group,
            PlayerManager* pPlayerManager,
            UserSettingsPointer pConfig,
            QWidget* parent = nullptr);

  private:
    bool drawNextPixmapPart() override;
};

#endif // WOVERVIEWRGB_H
