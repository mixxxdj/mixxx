#pragma once

class Track;
class QMenu;

namespace mixxx {

namespace library {

void createFindOnWebSubmenus(QMenu* pFindOnWebMenu, const Track& track);

} // namespace library

} // namespace mixxx
