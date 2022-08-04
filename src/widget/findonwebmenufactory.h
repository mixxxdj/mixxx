#pragma once

#include <QMenu>

class Track;

namespace mixxx {

namespace library {

void createFindOnWebSubmenus(QMenu* pFindOnWebMenu, const Track& track);

} // namespace library

} // namespace mixxx
