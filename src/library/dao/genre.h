#pragma once

#include <QMetaType>
#include <QString>

#include "util/db/dbid.h"

struct Genre {
    DbId id;
    QString name;

    Genre() = default;
    Genre(DbId genreId, const QString& genreName)
            : id(genreId),
              name(genreName) {
    }

    bool isValid() const {
        return id.isValid() && !name.isEmpty();
    }

    bool operator==(const Genre& other) const {
        return id == other.id && name == other.name;
    }

    bool operator!=(const Genre& other) const {
        return !(*this == other);
    }
};

Q_DECLARE_METATYPE(Genre)

// Allows printing a list of Genres directly to a QDebug stream.
QDebug operator<<(QDebug dbg, const QList<Genre>& genres);
