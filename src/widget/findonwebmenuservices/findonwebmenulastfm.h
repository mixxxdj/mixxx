#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;

class FindOnWebMenuLastfm : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuLastfm(QMenu* pFindOnWebMenu, const Track& track);
};
