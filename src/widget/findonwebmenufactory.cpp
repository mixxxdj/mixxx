#include "findonwebmenufactory.h"

#include <QMenu>

#include "findonwebmenuservices/findonwebmenudiscogs.h"
#include "findonwebmenuservices/findonwebmenulastfm.h"
#include "findonwebmenuservices/findonwebmenusoundcloud.h"
#include "util/parented_ptr.h"

namespace mixxx {

namespace library {

void createFindOnWebSubmenus(QMenu* pFindOnWebMenu, const Track& track) {
    make_parented<QMenu>(new FindOnWebMenuSoundcloud(pFindOnWebMenu, track));
    make_parented<QMenu>(new FindOnWebMenuDiscogs(pFindOnWebMenu, track));
    make_parented<QMenu>(new FindOnWebMenuLastfm(pFindOnWebMenu, track));
}

} // namespace library

} // namespace mixxx
