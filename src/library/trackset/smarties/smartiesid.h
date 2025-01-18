#pragma once

#include "util/db/dbid.h"

class SmartiesId : public DbId {
  public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(SmartiesId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(SmartiesId)
