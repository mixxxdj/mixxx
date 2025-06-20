#ifndef GENRE_H
#define GENRE_H

#include <QString>

#include "util/db/dbid.h"

// Represents a single genre entity from the database.
// This struct is used to pass genre data between the DAO and other Mixxx parts.
struct Genre {
    DbId id = DbId_Invalid;
    QString name;

    // Overload operator== for easy comparison, e.g. in QList::contains()
    bool operator==(const Genre& other) const {
        return id == other.id;
    }
};

#endif
