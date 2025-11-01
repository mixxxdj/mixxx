#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;
class FindOnWebLast;

class FindOnWebMenuSpotify : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuSpotify(QMenu* pFindOnWebMenu, FindOnWebLast* pFindOnWebLast, const Track& track);
};
