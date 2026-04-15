#pragma once

#include "QPointer"

class Track;
class QMenu;
class FindOnWebLast;

namespace mixxx {

namespace library {

void createFindOnWebSubmenus(const QPointer<QMenu>& pFindOnWebMenu,
        const QPointer<FindOnWebLast>& pFindOnWebLast,
        const Track& track);

} // namespace library

} // namespace mixxx
