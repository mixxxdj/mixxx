#ifndef WTREEMINIVIEW_H
#define WTREEMINIVIEW_H
#include "widget/wminiviewscrollbar.h"

class WTreeMiniView : public WMiniViewScrollBar
{
  public:
    WTreeMiniView(QWidget* parent = nullptr);

  protected:
    void refreshCharMap() override;
};

#endif // WTREEMINIVIEW_H
