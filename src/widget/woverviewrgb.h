#pragma once

#include "widget/woverview.h"

class WOverviewRGB : public WOverview {
    Q_OBJECT
  public:
    WOverviewRGB(
            const QString& group,
            PlayerManager* pPlayerManager,
            UserSettingsPointer pConfig,
            QWidget* parent = nullptr);

  private:
    bool drawNextPixmapPart() override;
};
