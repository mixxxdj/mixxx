#pragma once

#include "track/track.h"
#include "util/parented_ptr.h"
#include "widget/wfindonwebmenu.h"

class FindOnWebMenuSoundcloud : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuSoundcloud(QMenu* pFindOnWebMenu, const Track& track);
};
