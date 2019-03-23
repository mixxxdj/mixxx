#ifndef MIXXX_CRATEID_H
#define MIXXX_CRATEID_H


#include "util/db/dbid.h"


class CrateId: public DbId {
public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(CrateId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(CrateId)


#endif // MIXXX_CRATEID_H
