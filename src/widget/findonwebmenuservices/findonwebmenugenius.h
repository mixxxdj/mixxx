#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;
class FindOnWebLast;

class FindOnWebMenuGenius : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuGenius(QMenu* pFindOnWebMenu, FindOnWebLast* pFindOnWebLast, const Track& track);
};
