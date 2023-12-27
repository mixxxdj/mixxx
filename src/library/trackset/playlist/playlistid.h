#pragma once

#include "util/db/dbid.h"

class PlaylistId : public DbId {
  public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(PlaylistId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(PlaylistId)
