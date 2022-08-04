#pragma once

#include "track/track.h"
#include "util/parented_ptr.h"
#include "widget/wfindonwebmenu.h"

class FindOnWebMenuDiscogs : public WFindOnWebMenu {
  public:
    FindOnWebMenuDiscogs(QMenu* pFindOnWebMenu, const Track& track);
};
