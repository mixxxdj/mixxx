#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;
class FindOnWebLast;

class FindOnWebMenuWikipedia : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuWikipedia(QMenu* pFindOnWebMenu,
            FindOnWebLast* pFindOnWebLast,
            const Track& track);
};
