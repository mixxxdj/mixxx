#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;
class FindOnWebLast;

class FindOnWebMenuWikipedia : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuWikipedia(const QPointer<QMenu>& pFindOnWebMenu,
            QPointer<FindOnWebLast> pFindOnWebLast,
            const Track& track);
};
