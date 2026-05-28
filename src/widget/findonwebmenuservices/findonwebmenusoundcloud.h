#pragma once

#include "widget/wfindonwebmenu.h"

class Track;
class FindOnWebLast;

class FindOnWebMenuSoundcloud : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuSoundcloud(const QPointer<QMenu>& pFindOnWebMenu,
            QPointer<FindOnWebLast> pFindOnWebLast,
            const Track& track);
};
