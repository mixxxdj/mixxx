#ifndef WOVERVIEWRGB_H
#define WOVERVIEWRGB_H

#include "widget/woverview.h"

class WOverviewRGB : public WOverview {
  public:
    WOverviewRGB(const char *pGroup, UserSettingsPointer pConfig, QWidget* parent);

  private:
    virtual bool drawNextPixmapPart();
};

#endif // WOVERVIEWRGB_H
