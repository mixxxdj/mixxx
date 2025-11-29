#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;
class FindOnWebLast;

class FindOnWebMenuTunebat : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuTunebat(QMenu* pFindOnWebMenu, FindOnWebLast* pFindOnWebLast, const Track& track);
};
