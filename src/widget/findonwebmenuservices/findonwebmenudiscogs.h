#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;

class FindOnWebMenuDiscogs : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuDiscogs(QMenu* pFindOnWebMenu, const Track& track);
};
