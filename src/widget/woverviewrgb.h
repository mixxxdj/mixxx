#ifndef WOVERVIEWRGB_H
#define WOVERVIEWRGB_H

#include "widget/woverview.h"

class WOverviewRGB : public WOverview {
  public:
    WOverviewRGB(const char *pGroup, UserSettingsPointer pConfig, QWidget* parent);

  private:
    bool drawNextPixmapPart() override;
};

#endif // WOVERVIEWRGB_H
