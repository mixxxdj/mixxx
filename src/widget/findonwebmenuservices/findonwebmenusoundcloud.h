#pragma once

#include "widget/wfindonwebmenu.h"

class Track;

class FindOnWebMenuSoundcloud : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuSoundcloud(QMenu* pFindOnWebMenu, const Track& track);
};
