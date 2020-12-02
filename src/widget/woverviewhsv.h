#ifndef WOVERVIEWHSV_H
#define WOVERVIEWHSV_H

#include <QString>

#include "preferences/usersettings.h"
#include "widget/woverview.h"

class PlayerManager;
class QWidget;

class WOverviewHSV : public WOverview {
  public:
    WOverviewHSV(
            const QString& group,
            PlayerManager* pPlayerManager,
            UserSettingsPointer pConfig,
            QWidget* parent = nullptr);

  private:
    bool drawNextPixmapPart() override;
};

#endif // WOVERVIEWHSV_H
