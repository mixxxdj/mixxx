#pragma once

#include "widget/wfindonwebmenu.h"

class Track;
class FindOnWebLast;

class FindOnWebMenuSoundcloud : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuSoundcloud(QMenu* pFindOnWebMenu,
            FindOnWebLast* pFindOnWebLast,
            const Track& track);
};
