#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;
class FindOnWebLast;

class FindOnWebMenuDiscogs : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuDiscogs(QMenu* pFindOnWebMenu, FindOnWebLast* pFindOnWebLast, const Track& track);
};
