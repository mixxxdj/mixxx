#ifndef WOVERVIEWLMH_H
#define WOVERVIEWLMH_H

#include <QString>

#include "preferences/usersettings.h"
#include "widget/woverview.h"

class PlayerManager;
class QWidget;

class WOverviewLMH : public WOverview {
  public:
    WOverviewLMH(
            const QString& group,
            PlayerManager* pPlayerManager,
            UserSettingsPointer pConfig,
            QWidget* parent = nullptr);

  private:
    bool drawNextPixmapPart() override;
};

#endif // WOVERVIEWLMH_H
