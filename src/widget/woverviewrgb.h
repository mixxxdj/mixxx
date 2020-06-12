#ifndef WOVERVIEWRGB_H
#define WOVERVIEWRGB_H

#include "widget/woverview.h"

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
