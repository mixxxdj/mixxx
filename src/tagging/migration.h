#pragma once

// FIXME: Delete this migrational header file in final version!

#include "library/tags/facets.h"

namespace mixxx {

// Imported types
using library::tags::Facets;
using library::tags::Tag;
using library::tags::TagVector;

// Imported and renamed types
using TagFacetId = library::tags::FacetId;
using TagLabel = library::tags::Label;
using TagScore = library::tags::Score;

} // namespace mixxx
