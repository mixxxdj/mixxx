/// Utilities for iterating through a selection or collection
/// of tracks.

#pragma once

#include <QModelIndex>

#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/itemiterator.h"

namespace mixxx {

typedef ItemIterator<TrackId> TrackIdIterator;
typedef ListItemIterator<TrackId> TrackIdListIterator;

typedef ItemIterator<TrackPointer> TrackPointerIterator;
typedef ListItemIterator<TrackPointer> TrackPointerListIterator;

} // namespace mixxx
