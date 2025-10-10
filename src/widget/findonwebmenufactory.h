#pragma once

class Track;
class QMenu;
class FindOnWebLast;

namespace mixxx {

namespace library {

void createFindOnWebSubmenus(QMenu* pFindOnWebMenu,
        FindOnWebLast* pFindOnWebLast,
        const Track& track);

} // namespace library

} // namespace mixxx
