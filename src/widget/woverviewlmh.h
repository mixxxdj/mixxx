#ifndef WOVERVIEWLMH_H
#define WOVERVIEWLMH_H

#include "widget/woverview.h"

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
