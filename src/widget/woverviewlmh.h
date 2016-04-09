#ifndef WOVERVIEWLMH_H
#define WOVERVIEWLMH_H

#include "widget/woverview.h"

class WOverviewLMH : public WOverview {
  public:
    WOverviewLMH(const char *pGroup, UserSettingsPointer pConfig, QWidget* parent);

  private:
    virtual bool drawNextPixmapPart();
};

#endif // WOVERVIEWLMH_H
