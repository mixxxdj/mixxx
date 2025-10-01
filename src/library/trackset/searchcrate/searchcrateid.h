#pragma once

#include "util/db/dbid.h"

class SearchCrateId : public DbId {
  public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(SearchCrateId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(SearchCrateId)
