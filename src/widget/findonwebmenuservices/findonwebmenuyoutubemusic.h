#pragma once

#include "widget/wfindonwebmenu.h"

class QMenu;
class Track;
class FindOnWebLast;

class FindOnWebMenuYouTubeMusic : public WFindOnWebMenu {
    Q_OBJECT
  public:
    FindOnWebMenuYouTubeMusic(const QPointer<QMenu>& pFindOnWebMenu,
            QPointer<FindOnWebLast> pFindOnWebLast,
            const Track& track);
};
