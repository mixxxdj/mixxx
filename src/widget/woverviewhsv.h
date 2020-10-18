#ifndef WOVERVIEWHSV_H
#define WOVERVIEWHSV_H

#include "widget/woverview.h"

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
