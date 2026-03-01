#include "findonwebmenufactory.h"

#include <QMenu>

#include "findonwebmenuservices/findonwebmenudiscogs.h"
#include "findonwebmenuservices/findonwebmenulastfm.h"
#include "findonwebmenuservices/findonwebmenusoundcloud.h"
#include "util/parented_ptr.h"

namespace mixxx {

namespace library {

void createFindOnWebSubmenus(const QPointer<QMenu>& pFindOnWebMenu,
        const QPointer<FindOnWebLast>& pFindOnWebLast,
        const Track& track) {
    make_parented<FindOnWebMenuDiscogs>(pFindOnWebMenu, pFindOnWebLast, track);
    make_parented<FindOnWebMenuLastfm>(pFindOnWebMenu, pFindOnWebLast, track);
    make_parented<FindOnWebMenuSoundcloud>(pFindOnWebMenu, pFindOnWebLast, track);
}

} // namespace library

} // namespace mixxx
