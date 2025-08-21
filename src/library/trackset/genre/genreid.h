#pragma once

#include "util/db/dbid.h"

class GenreId : public DbId {
  public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(GenreId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(GenreId)
