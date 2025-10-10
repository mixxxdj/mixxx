#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;
class FindOnWebLast;

class FindOnWebMenuLastfm : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuLastfm(QMenu* pFindOnWebMenu, FindOnWebLast* pFindOnWebLast, const Track& track);
};
