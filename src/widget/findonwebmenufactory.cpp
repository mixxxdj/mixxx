#include "findonwebmenufactory.h"

#include <QMenu>

#include "findonwebmenuservices/findonwebmenudiscogs.h"
#include "findonwebmenuservices/findonwebmenugenius.h"
#include "findonwebmenuservices/findonwebmenulastfm.h"
#include "findonwebmenuservices/findonwebmenusoundcloud.h"
#include "findonwebmenuservices/findonwebmenuspotify.h"
#include "findonwebmenuservices/findonwebmenutunebat.h"
#include "findonwebmenuservices/findonwebmenuwikipedia.h"
#include "findonwebmenuservices/findonwebmenuyoutubemusic.h"
#include "util/parented_ptr.h"

namespace mixxx {

namespace library {

void createFindOnWebSubmenus(const QPointer<QMenu>& pFindOnWebMenu,
        const QPointer<FindOnWebLast>& pFindOnWebLast,
        const Track& track) {
    make_parented<FindOnWebMenuDiscogs>(pFindOnWebMenu, pFindOnWebLast, track);
    make_parented<FindOnWebMenuLastfm>(pFindOnWebMenu, pFindOnWebLast, track);
    make_parented<FindOnWebMenuSoundcloud>(pFindOnWebMenu, pFindOnWebLast, track);
    make_parented<FindOnWebMenuWikipedia>(pFindOnWebMenu, pFindOnWebLast, track);
    make_parented<FindOnWebMenuYouTubeMusic>(pFindOnWebMenu, pFindOnWebLast, track);
    make_parented<FindOnWebMenuSpotify>(pFindOnWebMenu, pFindOnWebLast, track);
    make_parented<FindOnWebMenuGenius>(pFindOnWebMenu, pFindOnWebLast, track);
    make_parented<FindOnWebMenuTunebat>(pFindOnWebMenu, pFindOnWebLast, track);
}

} // namespace library

} // namespace mixxx
