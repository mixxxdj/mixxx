#ifndef TRACKID_H
#define TRACKID_H


#include "util/dbid.h"


class TrackId: public DbId {
public:
#if defined(_MSC_VER) && (_MSC_VER < 1900)
    // NOTE(uklotzde): Inheriting constructors are supported since VS2015.
    // Use a default constructor and a single-argument constructor with
    // perfect forwarding instead.
    TrackId() {}
    template<typename T>
    explicit TrackId(T&& arg): DbId(std::forward<T>(arg)) {}
#else
    // Inherit constructors from base class
    using DbId::DbId;
#endif
};

Q_DECLARE_METATYPE(TrackId)


#endif // TRACKID_H
