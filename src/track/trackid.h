#ifndef TRACKID_H
#define TRACKID_H


#include "util/db/dbid.h"


class TrackId: public DbId {
public:
    // Inherit constructors from base class
    using DbId::DbId;
};

Q_DECLARE_TYPEINFO(TrackId, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TrackId)


#endif // TRACKID_H
