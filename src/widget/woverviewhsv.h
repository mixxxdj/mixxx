#pragma once

#include "widget/woverview.h"

class WOverviewHSV : public WOverview {
    Q_OBJECT
  public:
    WOverviewHSV(
            const QString& group,
            PlayerManager* pPlayerManager,
            UserSettingsPointer pConfig,
            QWidget* parent = nullptr);

  private:
    bool drawNextPixmapPart() override;
};
