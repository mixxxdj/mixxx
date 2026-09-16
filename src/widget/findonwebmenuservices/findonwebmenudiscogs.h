#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;
class FindOnWebLast;

class FindOnWebMenuDiscogs : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuDiscogs(const QPointer<QMenu>& pFindOnWebMenu,
            QPointer<FindOnWebLast> pFindOnWebLast,
            const Track& track);
};
