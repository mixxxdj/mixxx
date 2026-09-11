#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;
class FindOnWebLast;

class FindOnWebMenuSpotify : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuSpotify(const QPointer<QMenu>& pFindOnWebMenu,
            QPointer<FindOnWebLast> pFindOnWebLast,
            const Track& track);
};
